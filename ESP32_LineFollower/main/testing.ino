#include <Arduino.h>
#include <math.h>
#include "telemetry_api.h"

float KP{0.5}, KD{0.1}, KI{0.00};
float previousError{0}, errorSum{0};

int speed{100};
bool startPID{true};

void doPID(float error, bool onTheLine) {
    const unsigned long controlStartUs = micros();

    if (!onTheLine) error = constrain(previousError, -8, 8);

    float errorDiff = error - previousError;
    errorSum += error;
    errorSum = constrain(errorSum, -100, 100);
    int correction = static_cast<int>((KP * error + KD * errorDiff + KI * errorSum));
    previousError = error;

    publishLineError(error);
    publishMotorCommands(speed - correction, speed + correction);
    publishControlLoopTime(micros() - controlStartUs);
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting up");

    robotSettings = RobotSettings{KP, KI, KD, speed, 255};
    beginTelemetry("HERMES-ROBOT", "Hermes123");
}

void loop() {
    serviceTelemetry();

    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate >= 20) {
        lastUpdate = millis();

        KP = robotSettings.kp;
        KI = robotSettings.ki;
        KD = robotSettings.kd;
        speed = robotSettings.baseSpeed;

        // Fake sensors, so the website has something to show.
        float t = millis() / 1000.0f;
        uint16_t fakeSensors[16];
        for (int i = 0; i < 16; i++) {
            fakeSensors[i] = 2048 + 2000 * sinf(t * 2 + i * 0.4f);
        }
        publishSensorValues(fakeSensors);

        float linePos = 40 * sinf(t);
        if (startPID) {
            doPID(linePos, true);
        }
    }
}