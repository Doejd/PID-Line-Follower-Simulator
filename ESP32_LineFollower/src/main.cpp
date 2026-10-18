#include <Arduino.h>

#include "sensors.h"
#include "motors.h"

void setup() {
    // Serial.begin(115200);

  // sensorsSetup();
  motorsSetup();
}


void loop() {
    setMotor1Speed(100);
    setMotor2Speed(100);

    delay(20);
}
