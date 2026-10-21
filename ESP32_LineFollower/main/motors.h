#include <Arduino.h>
//Motor 1
const int AIN1 = 1; //(Pin 1)
const int AIN2 = 2; //(Pin 2)
//Motor 2
const int BIN1 = 6; //(Pin 6)
const int BIN2 = 7; //(Pin 7)

const int MAX_SPEED = 255;
const int MIN_SPEED = -150;


// Using slow decay (fast acceleration/deceleration) -> speed needs to be inverted
void moveForward(int speed, int in1, int in2) { 
    analogWrite(in1, 255);
    analogWrite(in2, 255 - speed);
}
void moveBackward(int speed, int in1, int in2) {
    analogWrite(in1, 255 - speed);
    analogWrite(in2, 255);
}
void brake(int in1, int in2) {
    analogWrite(in1, 255);
    analogWrite(in2, 255);
}
void coast(int in1, int in2) {
    analogWrite(in1, 0);
    analogWrite(in2, 0);
}

void motorsSetup() {
    pinMode(AIN1, OUTPUT);
    pinMode(AIN2, OUTPUT);
    pinMode(BIN1, OUTPUT);
    pinMode(BIN2, OUTPUT);

    coast(AIN1, AIN2);
    coast(BIN1, BIN2);
}

void writeToMotor(int speed, int in1,int in2) {
    speed = constrain(speed, MIN_SPEED, MAX_SPEED);
    if(speed >= 0) moveForward(speed, in1, in2);
    else moveBackward(abs(speed), in1, in2);
}

void setMotorSpeeds(int leftSpeed, int rightSpeed){
    writeToMotor(leftSpeed, BIN1, BIN2);
    writeToMotor(rightSpeed, AIN1, AIN2);
}

void steer(int baseSpeed, int correction){
    int leftSpeed = baseSpeed + correction;
    int rightSpeed = baseSpeed - correction;
    setMotorSpeeds(leftSpeed, rightSpeed);
}
