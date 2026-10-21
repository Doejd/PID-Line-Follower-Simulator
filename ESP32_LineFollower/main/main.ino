#include <Arduino.h>
#include <array>
#include "sensors.h"
#include "motors.h"
#include "telemetry_api.h"
#include "display_ui.h"

float KP{0.5}, KD{0.1}, KI{0.00};
float previousError{0}, errorSum{0};

int speed{100};
bool startPID{false};

unsigned long calibrationEnd = 3000;


constexpr DisplayUiConfig displayUiConfig{
    OLED_SDA_GPIO,
    OLED_SCL_GPIO,
    BUTTON_1_GPIO,
    BUTTON_2_GPIO,
    BUTTON_3_GPIO,
    0x3C
};

void handleDisplayAction() {
    const DisplayUiAction action = consumeDisplayUiAction();

    if (action == DisplayUiAction::CalibrationRequested) {
        Serial.println("Calibration requested");
        // calibrationEnd = millis() + 3000;
    }
    else if (action == DisplayUiAction::StartRequested) {
        Serial.println("Start requeseted");
        // startPID = !startPID;
    }
}


void doPID(float error, bool onTheLine) {
    /*
     * This function implements a PID controller to adjust the robot's movement based on the calculated error.
     * It calculates the proportional, integral, and derivative components of the error and computes a correction.
     * TODO: Apply the correction term to the motors.
    */

    const unsigned long controlStartUs = micros();

    if (!onTheLine) error = constrain(previousError, -8, 8);
    
    float errorDiff = error - previousError;
    errorSum += error;
    errorSum = constrain(errorSum, -100, 100);
    int correction = static_cast<int>((KP * error + KD * errorDiff + KI * errorSum));
    // correction = constrain(correction, -70, 70);
    previousError = error;
    
    Serial.println(correction);
    steer(speed, correction);
    
    // publishSensorValues(sensorValues.data());
    // publishLineError(error);
    // publishMotorCommands(speed - correction, speed + correction);
    // publishControlLoopTime(micros() - controlStartUs);
}


void setup() {
    Serial.begin(115200);
    Serial.println("Starting up");

    // robotSettings = RobotSettings{KP, KI, KD, speed, 255};
    // beginTelemetry("HERMES-ROBOT", "Hermes123");

    sensorsSetup();
    motorsSetup();

    startPID = true;

    // if (!beginDisplayUi(displayUiConfig)) {
    //   Serial.println("Display initialization failed");
    // }
}

void loop() {
  static unsigned long lastUpdate = 0;
  
  // Run sensor update loop at roughly 50 Hz (every 20ms)
  if (millis() - lastUpdate >= 20) {
    lastUpdate = millis();
    // serviceTelemetry();

    // KP = robotSettings.kp;
    // KI = robotSettings.ki;
    // KD = robotSettings.kd;
    // speed = robotSettings.baseSpeed;
    // serviceDisplayUi();
    // handleDisplayAction();

    readAllSensors();
    if(millis() < calibrationEnd){
      calibrationStep();
    }

    if(startPID){
        float linePos = 0;
        bool onTheLine = getLinePosition(linePos);
    
        doPID(linePos, onTheLine);
    
        if (onTheLine) Serial.println(linePos);
    }
  }
}