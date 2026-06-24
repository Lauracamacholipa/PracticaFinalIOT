#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiManager.h>

#include "config.h"
#include "secrets.h"
#include "pump.h"
#include "sensor.h"

WiFiClientSecure wifiClient;
PubSubClient client(wifiClient);

String shadowUpdateTopic;
String shadowDeltaTopic;
String telemetryTopic;
String currentMode = "automatic";

int thresholdLow = 30;
int thresholdHigh = 70;

String getCurrentMode() {
    return currentMode;
}

int getThresholdLow() {
    return thresholdLow;
}

int getThresholdHigh() {
    return thresholdHigh;
}

void buildTopics() {
    String base = "$aws/things/" + String(THING_NAME) + "/shadow";

    shadowUpdateTopic = base + "/update";
    shadowDeltaTopic  = base + "/update/delta";

    telemetryTopic =
      "macetas/" + String(THING_NAME) + "/telemetry";
}

void callback(char* topic, byte* payload, unsigned int length)
{
    Serial.println("===== CALLBACK MQTT =====");

    String msg;

    for(int i = 0; i < length; i++){
        msg += (char)payload[i];
    }

    Serial.print("TOPIC RECIBIDO: ");
    Serial.println(topic);

    Serial.println("MQTT recibido:");
    Serial.println(msg);

    StaticJsonDocument<1024> doc;

    DeserializationError error = deserializeJson(doc, msg);

    if(error){
        Serial.print("Error JSON: ");
        Serial.println(error.c_str());
        return;
    }

    if(String(topic) == shadowDeltaTopic){

        JsonObject state = doc["state"].as<JsonObject>();

        // 1. Primero actualizar modo
        if(state.containsKey("mode")){
            currentMode = state["mode"].as<String>();

            Serial.print("Modo actualizado: ");
            Serial.println(currentMode);
        }

        // 2. Luego actualizar umbrales
        if(state.containsKey("thresholdLow")){
            int newLow = state["thresholdLow"].as<int>();

            if(newLow < thresholdHigh){
                thresholdLow = newLow;

                Serial.print("Threshold Low actualizado: ");
                Serial.println(thresholdLow);
            }
            else{
                Serial.println("Threshold Low rechazado: no puede ser mayor o igual al Threshold High");
            }
        }

        if(state.containsKey("thresholdHigh")){
            int newHigh = state["thresholdHigh"].as<int>();

            if(newHigh > thresholdLow){
                thresholdHigh = newHigh;

                Serial.print("Threshold High actualizado: ");
                Serial.println(thresholdHigh);
            }
            else{
                Serial.println("Threshold High rechazado: no puede ser menor o igual al Threshold Low");
            }
        }

        // 3. irrigation SOLO se aplica en modo manual
        if(state.containsKey("irrigation") && !state["irrigation"].isNull()){

            bool cmd = state["irrigation"].as<bool>();

            if(currentMode == "manual"){

                if(cmd){
                    turnPumpOn();
                    Serial.println("BOMBA ENCENDIDA POR SHADOW EN MODO MANUAL");
                }
                else{
                    turnPumpOff();
                    Serial.println("BOMBA APAGADA POR SHADOW EN MODO MANUAL");
                }

            }
            else{
                Serial.println("Comando irrigation ignorado porque el sistema está en modo automático");
            }
        }
    }
}
void connectWiFi() {

    WiFiManager wm;

    wm.resetSettings();

    bool res = wm.autoConnect("SmartPlantSetup");

    if(!res){
        Serial.println("No se pudo conectar");
        ESP.restart();
    }

    Serial.println("WiFi conectado");
}

void reconnectMQTT() {
    while(!client.connected()){

        Serial.println("Conectando AWS MQTT...");

        if(client.connect(CLIENT_ID)){
            Serial.println("MQTT conectado");

            bool subOk = client.subscribe(shadowDeltaTopic.c_str());

            Serial.print("Suscrito a delta: ");
            Serial.println(shadowDeltaTopic);

            Serial.print("Resultado subscribe: ");
            Serial.println(subOk ? "OK" : "FALLÓ");
        }
        else{
            Serial.println(client.state());
            delay(5000);
        }
    }
}

void initAWS() {
    buildTopics();

    connectWiFi();

    wifiClient.setCACert(
        AMAZON_ROOT_CA1
    );

    wifiClient.setCertificate(
        CERTIFICATE
    );

    wifiClient.setPrivateKey(
        PRIVATE_KEY
    );

    client.setServer(
        MQTT_BROKER,
        MQTT_PORT
    );

    client.setBufferSize(1024);

    client.setCallback(callback);
}

void loopAWS() {
    if(!client.connected())
        reconnectMQTT();

    client.loop();
}

void publishTelemetry(int humidity, bool irrigation) {
    if (!client.connected()) {
        return;
    }

    bool sensorOk = isSensorOk();

    String ledStatus = "normal";

    if (!sensorOk) {
        ledStatus = "error";
    }
    else if (irrigation) {
        ledStatus = "irrigating";
    }
    else if (humidity < thresholdLow) {
        ledStatus = "dry";
    }
    else if (humidity > thresholdHigh) {
        ledStatus = "wet";
    }
    else {
        ledStatus = "normal";
    }

    StaticJsonDocument<512> doc;

    doc["thing_name"] = THING_NAME;
    doc["humidity"] = humidity;
    doc["sensor_ok"] = sensorOk;
    doc["mode"] = currentMode;
    doc["irrigation"] = irrigation;
    doc["threshold_low"] = thresholdLow;
    doc["threshold_high"] = thresholdHigh;
    doc["raw_adc"] = getLastRawAdc();
    doc["wifi_rssi"] = WiFi.RSSI();
    doc["led_status"] = ledStatus;

    char buffer[512];
    serializeJson(doc, buffer);

    client.publish(telemetryTopic.c_str(), buffer);

    Serial.println("Telemetry:");
    Serial.println(buffer);

    StaticJsonDocument<512> shadowDoc;
    JsonObject state = shadowDoc.createNestedObject("state");
    JsonObject reported = state.createNestedObject("reported");

    reported["mode"] = currentMode;
    reported["irrigation"] = irrigation;
    reported["humidity"] = humidity;
    reported["sensor_ok"] = sensorOk;
    reported["raw_adc"] = getLastRawAdc();
    reported["thresholdLow"] = thresholdLow;
    reported["thresholdHigh"] = thresholdHigh;
    reported["thing_name"] = THING_NAME;
    reported["led_status"] = ledStatus;

    char shadowBuffer[512];
    serializeJson(shadowDoc, shadowBuffer);

    client.publish(shadowUpdateTopic.c_str(), shadowBuffer);

    Serial.println("Shadow update:");
    Serial.println(shadowBuffer);
}