#include <Arduino.h>
//Motor 1
const int AIN1 = 1; //(Pin 1)
const int AIN2 = 2; //(Pin 2)
//Motor 2
const int BIN1 = 6; //(Pin 6)
const int BIN2 = 7; //(Pin 7)


void motorsSetup() {
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);
}

// Using slow decay (fast acceleration/deceleration) -> speed needs to be inverted
void brake(int in1, int in2) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, HIGH);
}
void moveForward(int speed, int in1, int in2) {
    analogWrite(in1, 255 - speed);
    digitalWrite(in2, HIGH);
}
void moveBackward(int speed, int in1, int in2) { // speed > 0
    digitalWrite(in1, HIGH);
    analogWrite(in2, 255 - speed);
}

void writeToMotor(int speed, int in1,int in2) {
    speed = constrain(speed, -255, 255);
    if(speed == 0) brake(in1, in2);
    else if(speed > 0) moveForward(speed, in1, in2);
    else moveBackward(abs(speed), in1, in2);
}

void setMotor1Speed(int speed) {
    writeToMotor(speed, AIN1, AIN2);
}
void setMotor2Speed(int speed) {
    writeToMotor(speed, BIN1, BIN2);
}