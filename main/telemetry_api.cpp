#include "telemetry_api.h"

#include <ArduinoJson.h>
#include <LittleFS.h>
#include <WebServer.h>
#include <WiFi.h>
#include <math.h>

// These objects are shared with the existing main program.
RobotSettings robotSettings;
TelemetryData telemetryData;

namespace {

constexpr size_t MAX_SETTINGS_BODY_BYTES = 256;
// Temporary interface limit only; the PID team must set a tested gain range.
constexpr float MAX_PID_GAIN = 1000.0f;
constexpr int MAX_PWM_COMMAND = 255;

WebServer server(80);
bool serverStarted = false;
bool accessPointStarted = false;
bool dashboardFileSystemReady = false;

void handleDashboard() {
  if (!dashboardFileSystemReady) {
    server.send(503, "text/plain; charset=utf-8",
                "Dashboard storage is unavailable. Upload the LittleFS image.");
    return;
  }

  File dashboard = LittleFS.open("index.html", "r");
  if (!dashboard) {
    server.send(404, "text/plain; charset=utf-8",
                "Dashboard file index.html was not found.");
    return;
  }

  server.streamFile(dashboard, "text/html; charset=utf-8");
  dashboard.close();
}

void sendJsonError(int statusCode, const char* message) {
  JsonDocument response;
  response["error"] = message;

  String body;
  body.reserve(128);
  serializeJson(response, body);

  server.send(statusCode, "application/json", body);
}

bool readJsonNumber(JsonObjectConst object, const char* key, float& result) {
  JsonVariantConst value = object[key];

  // Do not accept missing values, strings, booleans, arrays, or objects.
  if (value.isNull() || value.is<bool>() || value.is<const char*>() ||
      value.is<JsonArrayConst>() || value.is<JsonObjectConst>() ||
      !value.is<float>()) {
    return false;
  }

  result = value.as<float>();
  return isfinite(result);
}

void writeSettings(JsonObject settings) {
  settings["kp"] = robotSettings.kp;
  settings["ki"] = robotSettings.ki;
  settings["kd"] = robotSettings.kd;
  settings["baseSpeed"] = robotSettings.baseSpeed;
  settings["maxSpeed"] = robotSettings.maxSpeed;
}

String settingsJson() {
  JsonDocument response;
  writeSettings(response.to<JsonObject>());

  String body;
  body.reserve(128);
  serializeJson(response, body);
  return body;
}

void handleGetSettings() {
  server.send(200, "application/json", settingsJson());
}

void handlePostSettings() {
  const String body = server.arg("plain");

  if (body.length() == 0 || body.length() > MAX_SETTINGS_BODY_BYTES) {
    sendJsonError(400, "Settings request is empty or too large.");
    return;
  }

  JsonDocument request;
  const DeserializationError parseError = deserializeJson(
      request, body, DeserializationOption::NestingLimit(2));
  if (parseError) {
    sendJsonError(400, "Settings must be a valid JSON object.");
    return;
  }

  const JsonObjectConst settings = request.as<JsonObjectConst>();
  if (settings.isNull()) {
    sendJsonError(400, "Settings must be a JSON object.");
    return;
  }

  float kp = 0;
  float ki = 0;
  float kd = 0;
  float baseSpeed = 0;
  float maxSpeed = 0;

  const bool hasAllNumbers = readJsonNumber(settings, "kp", kp) &&
                             readJsonNumber(settings, "ki", ki) &&
                             readJsonNumber(settings, "kd", kd) &&
                             readJsonNumber(settings, "baseSpeed", baseSpeed) &&
                             readJsonNumber(settings, "maxSpeed", maxSpeed);
  if (!hasAllNumbers) {
    sendJsonError(400, "Expected numeric kp, ki, kd, baseSpeed, and maxSpeed.");
    return;
  }

  const bool gainsInRange = kp >= 0 && kp <= MAX_PID_GAIN &&
                            ki >= 0 && ki <= MAX_PID_GAIN &&
                            kd >= 0 && kd <= MAX_PID_GAIN;
  const bool speedsInRange = baseSpeed >= 0 && baseSpeed <= MAX_PWM_COMMAND &&
                             maxSpeed >= 0 && maxSpeed <= MAX_PWM_COMMAND &&
                             floorf(baseSpeed) == baseSpeed &&
                             floorf(maxSpeed) == maxSpeed &&
                             baseSpeed <= maxSpeed;
  if (!gainsInRange || !speedsInRange) {
    sendJsonError(422, "Values are outside the allowed gain or speed ranges.");
    return;
  }

  // Validate the full request first, then replace the settings together.
  RobotSettings updatedSettings;
  updatedSettings.kp = kp;
  updatedSettings.ki = ki;
  updatedSettings.kd = kd;
  updatedSettings.baseSpeed = static_cast<int>(baseSpeed);
  updatedSettings.maxSpeed = static_cast<int>(maxSpeed);
  robotSettings = updatedSettings;

  server.send(200, "application/json", settingsJson());
}

void handleGetTelemetry() {
  JsonDocument response;
  JsonArray sensors = response["sensors"].to<JsonArray>();
  JsonArray rawSensors = response["rawSensors"].to<JsonArray>();

  for (int i = 0; i < 16; i++) {
    if (telemetryData.hasSensors) {
      sensors.add(telemetryData.sensors[i]);
      rawSensors.add(telemetryData.rawSensors[i]);
    } else {
      sensors.add(nullptr);
      rawSensors.add(nullptr);
    }
  }

  if (telemetryData.hasSensors) {
    response["sensorSequence"] = telemetryData.sensorSequence;
    response["sensorAgeMs"] = millis() - telemetryData.lastSensorUpdateMs;
  } else {
    response["sensorSequence"] = nullptr;
    response["sensorAgeMs"] = nullptr;
  }

  if (telemetryData.hasLineError) {
    response["lineError"] = telemetryData.lineError;
  } else {
    response["lineError"] = nullptr;
  }
  if (telemetryData.hasMotorValues) {
    response["motor1Command"] = telemetryData.motor1;
    response["motor2Command"] = telemetryData.motor2;
  } else {
    response["motor1Command"] = nullptr;
    response["motor2Command"] = nullptr;
  }

  response["elapsedMs"] = millis();
  if (telemetryData.hasControlLoopTime) {
    response["controlLoopUs"] = telemetryData.controlLoopUs;
  } else {
    response["controlLoopUs"] = nullptr;
  }

  const bool wifiConnected = WiFi.status() == WL_CONNECTED;
  response["wifiConnected"] = wifiConnected;
  if (wifiConnected) {
    response["wifiRssi"] = WiFi.RSSI();
  } else {
    response["wifiRssi"] = nullptr;
  }
  response["freeHeap"] = ESP.getFreeHeap();
  response["speedCmS"] = nullptr;
  response["batteryV"] = nullptr;

  JsonObject settings = response["settings"].to<JsonObject>();
  writeSettings(settings);

  String body;
  body.reserve(512);
  serializeJson(response, body);

  server.send(200, "application/json", body);
}

}  // namespace

void publishSensorValues(const uint16_t raw[16]) {
  for (int i = 0; i < 16; i++) {
    telemetryData.rawSensors[i] = raw[i];
    telemetryData.sensors[i] = constrain((raw[i] * 1000L) / 4095, 0, 1000);
  }

  telemetryData.hasSensors = true;
  telemetryData.sensorSequence++;
  telemetryData.lastSensorUpdateMs = millis();
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
  telemetryData.hasControlLoopTime = true;
}

void beginTelemetry(const char* ssid, const char* password) {
  dashboardFileSystemReady = LittleFS.begin(false);
  if (!dashboardFileSystemReady) {
    Serial.println("Dashboard LittleFS mount failed; upload the filesystem image.");
  }
  /* here i changed this dont delete
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(ssid, password);*/

  IPAddress localIP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);


  WiFi.mode(WIFI_AP);
  accessPointStarted = WiFi.softAP(ssid, password);

  if (accessPointStarted) {
    Serial.println("HERMES Wi-Fi started");
    Serial.print("Robot IP: ");
    Serial.println(WiFi.softAPIP());
  }else {
    Serial.println("ERROR: Wi-Fi AP failed");
  }

  server.on("/", HTTP_GET, handleDashboard);
  server.on("/api/telemetry", HTTP_GET, handleGetTelemetry);
  server.on("/api/settings", HTTP_GET, handleGetSettings);
  server.on("/api/settings", HTTP_POST, handlePostSettings);
}

void serviceTelemetry() {
  // Do not wait for Wi-Fi here: the control loop calls this function
  // repeatedly, so reconnect attempts remain asynchronous.
  if (WiFi.status() != WL_CONNECTED) {
    if (serverStarted) {
      server.stop();
      serverStarted = false;
    }
    return;
  }

 /*if (!serverStarted) {
    server.begin();
    serverStarted = true;
    Serial.print("Robot IP: ");
    Serial.println(WiFi.localIP());
  }

  if (serverStarted) {
    server.handleClient();
  }*/

  if (!accessPointStarted) {
    return;
  }

  if (!serverStarted) {
    server.begin();
    serverStarted = true;

    Serial.print("Dashboard IP: ");
    Serial.println(WiFi.softAPIP());
  }

  server.handleClient();
}

