#include <Arduino.h>
#include "config.h"

bool pumpState = false;

void turnPumpOff() {
    digitalWrite(IN1_MOTOR, LOW);
    digitalWrite(IN2_MOTOR, LOW);
    pumpState = false;
}

void initPump() {
    pinMode(IN1_MOTOR, OUTPUT);
    pinMode(IN2_MOTOR, OUTPUT);
    turnPumpOff();
}

void turnPumpOn() {
    digitalWrite(IN1_MOTOR, HIGH);
    digitalWrite(IN2_MOTOR, LOW);
    pumpState = true;
}

bool isPumpOn() {
    return pumpState;
}