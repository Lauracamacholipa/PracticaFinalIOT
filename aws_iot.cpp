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

String getCurrentMode() {
    return currentMode;
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

    for(int i = 0; i < length; i++)
        msg += (char)payload[i];

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

        if(state.containsKey("mode")){
            currentMode = state["mode"].as<String>();

            Serial.print("Modo actualizado: ");
            Serial.println(currentMode);
        }

        if(state.containsKey("irrigation")){
            bool cmd = state["irrigation"].as<bool>();

            if(cmd){
                turnPumpOn();
                Serial.println("BOMBA ENCENDIDA POR SHADOW");
            }
            else{
                turnPumpOff();
                Serial.println("BOMBA APAGADA POR SHADOW");
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

void publishTelemetry(int humidity, bool irrigation)
{
    if(!client.connected())
        return;

    StaticJsonDocument<256> telemetryDoc;

    telemetryDoc["thing_name"] = THING_NAME;
    telemetryDoc["humidity"] = humidity;
    telemetryDoc["irrigation"] = irrigation;

    char telemetryBuffer[256];
    serializeJson(telemetryDoc, telemetryBuffer);

    client.publish(
        telemetryTopic.c_str(),
        telemetryBuffer
    );

    StaticJsonDocument<256> shadowDoc;

    JsonObject state = shadowDoc.createNestedObject("state");
    JsonObject reported = state.createNestedObject("reported");

    reported["humidity"] = humidity;
    reported["irrigation"] = irrigation;
    reported["mode"] = currentMode;

    char shadowBuffer[256];
    serializeJson(shadowDoc, shadowBuffer);

    client.publish(
        shadowUpdateTopic.c_str(),
        shadowBuffer
    );

    Serial.println("Telemetry:");
    Serial.println(telemetryBuffer);

    Serial.println("Shadow update:");
    Serial.println(shadowBuffer);
}