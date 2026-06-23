#ifndef AWS_IOT_H
#define AWS_IOT_H

#include <Arduino.h>

void initAWS();
void loopAWS();
void publishTelemetry(
    int humidity,
    bool irrigation
);
String getCurrentMode();

#endif