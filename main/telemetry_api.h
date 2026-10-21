#pragma once
#include <Arduino.h>

// Settings the robot's main algorithm may read.
struct RobotSettings {
  // Safe placeholders. The algorithm team should choose its startup defaults.
  float kp = 0;
  float ki = 0;
  float kd = 0;
  int baseSpeed = 0;
  int maxSpeed = 0;
};

// Values the main robot program can publish to the website.
struct TelemetryData {
  int sensors[16] = {};
  uint16_t rawSensors[16] = {};
  bool hasSensors = false;
  uint32_t sensorSequence = 0;
  unsigned long lastSensorUpdateMs = 0;
  float lineError = 0;
  bool hasLineError = false;
  int motor1 = 0;
  int motor2 = 0;
  bool hasMotorValues = false;
  unsigned long controlLoopUs = 0;
  bool hasControlLoopTime = false;
};

extern RobotSettings robotSettings;
extern TelemetryData telemetryData;

void beginTelemetry(const char* ssid, const char* password);
void serviceTelemetry();
void publishSensorValues(const uint16_t raw[16]);
void publishMotorCommands(int motor1, int motor2);
void publishLineError(float error);
void publishControlLoopTime(unsigned long elapsedUs);
