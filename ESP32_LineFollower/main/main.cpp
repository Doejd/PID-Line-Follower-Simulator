#include <Arduino.h>
#include <array>
#include "Timer.h"
#include "sensors.h"
#include "motors.h"


float KP{20}, KD{22}, KI{0.05};
float previousError{0}, errorSum{0};
int sensorMin{0}, sensorMax{4096};
//  white          black

int speed{200};

std::array<uint16_t, 16> sensorValues{};
std::array<int16_t, 16> sensorWeights{-82, -71, -59, -46, -35, -25, -15, -5, 5,  15,  25,  35,  46,  59,  71, 82};
// std::array<int16_t, 16> sensorWeights{-8, -7, -6, -5, -4, -3, -2, -1, 1, 2, 3, 4, 5, 6, 7, 8};

void calibrateSensors() {
    /*
     * This function calibrates the sensors by reading their values multiple times while spinning and determining the minimum and maximum values.
     * The calibration process helps in accurately interpreting the sensor readings for line detection.
    */
    
    for (int i{0}; i < 3; ++i) {
        readSensorValues(sensorValues);
        
        sensorMin = min(static_cast<uint16_t>(sensorMin), *std::min_element(sensorValues.begin(), sensorValues.end()));
        sensorMax = max(static_cast<uint16_t>(sensorMax), *std::max_element(sensorValues.begin(), sensorValues.end()));
        
        delay(20);
    }
}

bool intToBlackAndWhite(int sensorValue) {
    /*
     * This function converts the sensor value to a binary representation of black and white.
     * true (1) represents black, and false (0) represents white. 
     * The conversion is based on the CALIBRATED minimum and maximum sensor values.
    */
    sensorValue = constrain(sensorValue, sensorMin, sensorMax);
    float range = static_cast<float>(sensorMax - sensorMin);
    float corrected = static_cast<float>(sensorValue - sensorMin);
    return round(corrected / range);
}

float calculateError() {
    /*
    * This function calculates the error based on the sensor readings and their corresponding weights.
    * The error is computed as a weighted sum of the sensor values, where each sensor contributes to the 
    * overall error based on its position relative to the line.
    */
    float error{0};
    readSensorValues(sensorValues);

    for (int i{0}; i < sensorValues.size(); ++i) {
        error += sensorWeights[i] * intToBlackAndWhite(sensorValues[i]);
    }

    return error;
}

bool isOnLine() {
    /*
    * This function checks if the robot is currently on the line by evaluating the sensor readings.
    * It returns true if any of the sensors detect the line (black), and false otherwise.
    */

    readSensorValues(sensorValues);

    for (const auto &val : sensorValues) {
        if (intToBlackAndWhite(val)) return true;
    }

    Serial.println("WARNING: Robot is off the line!");
    return false;
}


void doPID() {
    /*
     * This function implements a PID controller to adjust the robot's movement based on the calculated error.
     * It calculates the proportional, integral, and derivative components of the error and computes a correction.
     * TODO: Apply the correction term to the motors.
    */
    auto error = calculateError();

    if (!isOnLine()) error = constrain(previousError, -8, 8);

    float errorDiff = error - previousError;
    errorSum += error;
    errorSum = constrain(errorSum, -100, 100);
    float fCorrection = 0.1f * (KP * error + KD * errorDiff + KI * errorSum);
    int correction = static_cast<int>(fCorrection);
    previousError = error;
    
    Serial.println(correction);
    steer(speed, correction);
}

void setup() {
    Serial.begin(115200);

    sensorsSetup();
    motorsSetup();
}

void loop() {
    Timer timer{200};

    while (millis() < 15000) {
        timer.start();
        steer(0, 60);
        timer.onTimerTimeout(calibrateSensors);
    }

    timer.stop();

    doPID();

    delay(20);
}
