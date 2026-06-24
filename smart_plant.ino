#include "config.h"
#include "sensor.h"
#include "pump.h"
#include "leds.h"
#include "aws_iot.h"

int humidity = -1;

const unsigned long SENSOR_INTERVAL_MS = 30000;

const unsigned long AUTO_WATER_DURATION_MS = 5000;

const unsigned long AUTO_ABSORB_WAIT_MS = 30000;

unsigned long nextSensorReadMs = 0;
unsigned long autoStateStartMs = 0;

enum AutoWaterState {
    AUTO_IDLE,
    AUTO_WATERING,
    AUTO_ABSORBING
};

AutoWaterState autoState = AUTO_IDLE;

void setup() {
    Serial.begin(115200);

    initSensor();
    initPump();
    initLeds();
    initAWS();

    nextSensorReadMs = 0;
}

void handleManualMode(unsigned long now) {

    autoState = AUTO_IDLE;

    if(isPumpOn()){
        ledIrrigating();
    }
    else{
        setLeds(false, true);
    }

    if(now < nextSensorReadMs){
        return;
    }

    humidity = readHumidity();

    if(!isSensorOk()){
        if(!isPumpOn()){
            ledError();
        }

        publishTelemetry(
            -1,
            isPumpOn()
        );

        nextSensorReadMs = now + SENSOR_INTERVAL_MS;
        return;
    }

    publishTelemetry(
        humidity,
        isPumpOn()
    );

    nextSensorReadMs = now + SENSOR_INTERVAL_MS;
}

void handleAutomaticMode(unsigned long now) {

    if(autoState == AUTO_WATERING){

        ledIrrigating();

        if(now - autoStateStartMs >= AUTO_WATER_DURATION_MS){
            turnPumpOff();

            Serial.println("BOMBA APAGADA LOCALMENTE");
            Serial.println("Riego automático terminado. Esperando absorción.");

            publishTelemetry(
                humidity,
                isPumpOn()
            );

            loopAWS();

            autoState = AUTO_ABSORBING;
            autoStateStartMs = now;
        }

        return;
    }

    if(autoState == AUTO_ABSORBING){

        setLeds(false, true);

        if(now - autoStateStartMs >= AUTO_ABSORB_WAIT_MS){
            Serial.println("Tiempo de absorción terminado. Se hará nueva lectura.");

            autoState = AUTO_IDLE;
            nextSensorReadMs = 0;
        }

        return;
    }

    if(now < nextSensorReadMs){
        return;
    }

    humidity = readHumidity();

    if(!isSensorOk()){
        ledError();
        turnPumpOff();

        publishTelemetry(
            -1,
            isPumpOn()
        );

        nextSensorReadMs = now + SENSOR_INTERVAL_MS;
        return;
    }

    if(humidity < getThresholdLow()){

        ledDry();
        turnPumpOn();

        Serial.println("Humedad baja. Iniciando riego automático por 10 segundos.");

        autoState = AUTO_WATERING;
        autoStateStartMs = now;

        publishTelemetry(
            humidity,
            isPumpOn()
        );

        return;
    }

    if(humidity > getThresholdHigh()){
        ledWet();
        turnPumpOff();

        publishTelemetry(
            humidity,
            isPumpOn()
        );

        nextSensorReadMs = now + SENSOR_INTERVAL_MS;
        return;
    }

    setLeds(false, true);
    turnPumpOff();

    publishTelemetry(
        humidity,
        isPumpOn()
    );

    nextSensorReadMs = now + SENSOR_INTERVAL_MS;
}

void loop() {

    loopAWS();

    unsigned long now = millis();

    if(getCurrentMode() == "automatic"){
        handleAutomaticMode(now);
    }
    else{
        handleManualMode(now);
    }

    delay(20);
}