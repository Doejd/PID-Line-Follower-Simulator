#include <Arduino.h>

#include "sensors.h"
#include "motors.h"

void setup() {
    // Serial.begin(115200);

  // sensorsSetup();
  motorsSetup();
}




void loop() {
    // readSensorValues();

    // for (int i = 0; i < 16; i++) {
    //   Serial.print(sensorValues[i]);
    //   Serial.print("\t");
    // }
    // Serial.println();

    setMotor1Speed(100);
    setMotor2Speed(100);

    delay(20);
}
