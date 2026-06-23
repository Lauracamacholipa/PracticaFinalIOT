#ifndef LEDS_H
#define LEDS_H

void initLeds();
void setLeds(bool red, bool green);

void ledDry();
void ledWet();
void ledIrrigating();
void ledError();

#endif