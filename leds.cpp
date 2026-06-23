#include <Arduino.h>
#include "config.h"

bool blinkState = false;
unsigned long previousBlink = 0;

void initLeds() {
    pinMode(LED_ROJO, OUTPUT);
    pinMode(LED_VERDE, OUTPUT);

    digitalWrite(LED_ROJO, LOW);
    digitalWrite(LED_VERDE, LOW);
}

void setLeds(bool red, bool green){
    digitalWrite(LED_ROJO, red ? HIGH : LOW);
    digitalWrite(LED_VERDE, green ? HIGH : LOW);
}

void ledDry() {
    setLeds(true,false);
}

void ledWet() {
    setLeds(false,true);
}

void ledError() {
    unsigned long now = millis();

    if(now - previousBlink >= 400){
        previousBlink = now;
        blinkState = !blinkState;

        digitalWrite(LED_ROJO, blinkState);
        digitalWrite(LED_VERDE, !blinkState);
    }
}

void ledIrrigating() {
    unsigned long now = millis();

    if(now - previousBlink >= 400){
        previousBlink = now;
        blinkState = !blinkState;

        digitalWrite(LED_ROJO, blinkState);
        digitalWrite(LED_VERDE, LOW);
    }
}