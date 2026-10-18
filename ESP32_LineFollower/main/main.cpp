#include <Arduino.h>
#include <array>
#include "sensors.h"
#include "motors.h"

std::array<uint16_t, 16> sensorValues{};
void setup() {
    Serial.begin(115200);

  sensorsSetup();
  motorsSetup();
}

void loop() {
  readSensorValues(sensorValues);
  
  for (int i = 0; i < 16; i++) {
      Serial.print(sensorValues[i]);
      Serial.print("\t");
    }
    Serial.println();

    setMotor1Speed(100);
    setMotor2Speed(100);

    delay(20);
}
