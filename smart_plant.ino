#include "config.h"
#include "sensor.h"
#include "pump.h"
#include "leds.h"
#include "aws_iot.h"

int humidity;

void setup() {
    Serial.begin(115200);

    initSensor();
    initPump();
    initLeds();
    initAWS();
}

void loop() {

    loopAWS();

    humidity = readHumidity();

    if(!isSensorOk()){
        ledError();

        if(getCurrentMode() == "automatic"){
            turnPumpOff();
        }

        publishTelemetry(
            -1,
            isPumpOn()
        );

        for(int i = 0; i < 50; i++){
            loopAWS();
            delay(100);
        }

        return;
    }

    if(getCurrentMode() == "automatic"){
        if(humidity < 30){
            ledDry();
            turnPumpOn();
        }
        else if(humidity > 70){
            ledWet();
            turnPumpOff();
        }
        else{
            setLeds(false,true);
        }
    }
    else{
        if(isPumpOn()){
            ledIrrigating();
        }
        else{
            setLeds(false,true);
        }
    }

    publishTelemetry(
        humidity,
        isPumpOn()
    );

    for(int i = 0; i < 50; i++){
        loopAWS();
        delay(100);
    }
}