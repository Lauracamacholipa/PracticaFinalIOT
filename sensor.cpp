#include <Arduino.h>
#include "config.h"

bool sensorOk = false;
int lastRawAdc = -1;

void initSensor() {
    pinMode(SENSOR_HUMEDAD, INPUT);
    analogReadResolution(12);
}

bool isSensorOk() {
    return sensorOk;
}

int getLastRawAdc() {
    return lastRawAdc;
}

int readHumidity() {
    long sum = 0;

    for(int i = 0; i < 10; i++){
        sum += analogRead(SENSOR_HUMEDAD);
        delay(20);
    }

    int raw = sum / 10;
    lastRawAdc = raw;

    Serial.print("RAW: ");
    Serial.println(raw);

    if(raw <= ADC_ERROR_MIN || raw >= ADC_ERROR_MAX){
        sensorOk = false;
        return -1;
    }

    sensorOk = true;

    int humedad = map(raw,
                      SENSOR_SECO,
                      SENSOR_MOJADO,
                      0,
                      100);

    humedad = constrain(humedad, 0, 100);

    return humedad;
}