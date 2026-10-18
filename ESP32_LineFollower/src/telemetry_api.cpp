#include "telemetry_api.h"
#include <WiFi.h>
#include <WebServer.h>
#include <math.h>
#include <stdlib.h>

RobotSettings robotSettings;
TelemetryData telemetryData;

static WebServer server(80);
static bool serverStarted = false;

void publishSensorValues(const uint16_t raw[16]) {
  for (int i = 0; i < 16; i++) {
    telemetryData.rawSensors[i] = raw[i];
    telemetryData.sensors[i] = constrain((raw[i] * 1000L) / 4095, 0, 1000);
  }
  telemetryData.hasSensors = true;
}

void publishMotorCommands(int motor1, int motor2) {
  telemetryData.motor1 = motor1;
  telemetryData.motor2 = motor2;
  telemetryData.hasMotorValues = true;
}

void publishLineError(float error) {
  telemetryData.lineError = error;
  telemetryData.hasLineError = true;
}

void publishControlLoopTime(unsigned long elapsedUs) {
  telemetryData.controlLoopUs = elapsedUs;
}

static void addCorsHeaders() {
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Access-Control-Allow-Methods", "GET,POST,OPTIONS");
  server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

static void handleOptions() {
  addCorsHeaders();
  server.send(204);
}

static bool readJsonNumber(const String& body, const char* key, float& result) {
  String token = String("\"") + key + "\"";
  int start = body.indexOf(token);
  if (start < 0) return false;
  start = body.indexOf(':', start + token.length());
  if (start < 0) return false;
  start++;
  while (start < body.length() && isspace(body[start])) start++;

  int end = start;
  while (end < body.length()) {
    char c = body[end];
    if (!(isDigit(c) || c == '-' || c == '+' || c == '.' || c == 'e' || c == 'E')) break;
    end++;
  }
  if (end == start) return false;

  String value = body.substring(start, end);
  char* parsedEnd = nullptr;
  result = strtof(value.c_str(), &parsedEnd);
  return parsedEnd != value.c_str() && *parsedEnd == '\0' && isfinite(result);
}

static String settingsJson() {
  return String("{\"kp\":") + String(robotSettings.kp, 4) +
         ",\"ki\":" + String(robotSettings.ki, 4) +
         ",\"kd\":" + String(robotSettings.kd, 4) +
         ",\"baseSpeed\":" + robotSettings.baseSpeed +
         ",\"maxSpeed\":" + robotSettings.maxSpeed + "}";
}

static void handleGetSettings() {
  addCorsHeaders();
  server.send(200, "application/json", settingsJson());
}

static void handlePostSettings() {
  String body = server.arg("plain");
  float kp = robotSettings.kp;
  float ki = robotSettings.ki;
  float kd = robotSettings.kd;
  float baseSpeed = robotSettings.baseSpeed;
  float maxSpeed = robotSettings.maxSpeed;

  bool valid = readJsonNumber(body, "kp", kp) &&
               readJsonNumber(body, "ki", ki) &&
               readJsonNumber(body, "kd", kd) &&
               readJsonNumber(body, "baseSpeed", baseSpeed) &&
               readJsonNumber(body, "maxSpeed", maxSpeed);
  if (!valid) {
    addCorsHeaders();
    server.send(400, "application/json", "{\"error\":\"Expected kp, ki, kd, baseSpeed, maxSpeed\"}");
    return;
  }

  robotSettings.kp = constrain(kp, 0.0f, 1000.0f);
  robotSettings.ki = constrain(ki, 0.0f, 1000.0f);
  robotSettings.kd = constrain(kd, 0.0f, 1000.0f);
  robotSettings.baseSpeed = constrain((int)baseSpeed, 0, 255);
  robotSettings.maxSpeed = constrain((int)maxSpeed, 0, 255);

  addCorsHeaders();
  server.send(200, "application/json", settingsJson()); // acknowledgement to browser
}

static void handleGetTelemetry() {
  String json = "{\"sensors\":[";
  for (int i = 0; i < 16; i++) {
    if (i) json += ',';
    json += telemetryData.hasSensors ? String(telemetryData.sensors[i]) : "null";
  }
  json += "],\"rawSensors\":[";
  for (int i = 0; i < 16; i++) {
    if (i) json += ',';
    json += telemetryData.hasSensors ? String(telemetryData.rawSensors[i]) : "null";
  }
  json += "],\"lineError\":";
  json += telemetryData.hasLineError ? String(telemetryData.lineError, 3) : "null";
  json += ",\"motor1Command\":";
  json += telemetryData.hasMotorValues ? String(telemetryData.motor1) : "null";
  json += ",\"motor2Command\":";
  json += telemetryData.hasMotorValues ? String(telemetryData.motor2) : "null";
  json += ",\"elapsedMs\":" + String(millis());
  json += ",\"controlLoopUs\":";
  json += telemetryData.controlLoopUs ? String(telemetryData.controlLoopUs) : "null";
  json += ",\"wifiRssi\":" + String(WiFi.RSSI());
  json += ",\"freeHeap\":" + String(ESP.getFreeHeap());
  json += ",\"speedCmS\":null,\"batteryV\":null,";
  json += "\"settings\":" + settingsJson() + "}";
  addCorsHeaders();
  server.send(200, "application/json", json);
}

void beginTelemetry(const char* ssid, const char* password) {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password); // Start connecting; do not block the robot.
  server.on("/api/telemetry", HTTP_OPTIONS, handleOptions);
  server.on("/api/settings", HTTP_OPTIONS, handleOptions);
  server.on("/api/telemetry", HTTP_GET, handleGetTelemetry);
  server.on("/api/settings", HTTP_GET, handleGetSettings);
  server.on("/api/settings", HTTP_POST, handlePostSettings);
}

void serviceTelemetry() {
  if (!serverStarted && WiFi.status() == WL_CONNECTED) {
    server.begin();
    serverStarted = true;
    Serial.print("Robot IP: ");
    Serial.println(WiFi.localIP());
  }
  if (serverStarted) server.handleClient();
}
