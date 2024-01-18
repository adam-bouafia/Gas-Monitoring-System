#include "EspMQTTClient.h"
#include <Wire.h>
#include <DHTesp.h>
#include <AccelStepper.h>

EspMQTTClient client("Wokwi-GUEST", "", "test.mosquitto.org", "mqtt-mack-pub-sub", 1883);
#define DHTPIN 15
#define GAS_LEVER 34
#define TRIG_PIN 5
#define ECHO_PIN 18

DHTesp dhtsensor;
float duration_us, distance_cm;
const int motionSensor = 13;

AccelStepper stepper(AccelStepper::DRIVER, 19, 2, 4, 23);

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 collecting sensors data");

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(motionSensor, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);

    dhtsensor.setup(DHTPIN, DHTesp::DHT22);

    pinMode(GAS_LEVER, INPUT);
}

unsigned long lastSendTime = 0;
const unsigned long sendInterval = 2000;

void readSendData() {
    unsigned long currentTime = millis();
    if (currentTime - lastSendTime >= sendInterval) {
        TempAndHumidity data = dhtsensor.getTempAndHumidity();
        float gasLevel = map(analogRead(GAS_LEVER), 0, 4095, 0, 100);  
        String jsonData = parseSensorData(data.temperature, data.humidity, distance_cm, gasLevel);
        client.publish("univaq/gas", jsonData);
        lastSendTime = currentTime;
    }
}

void detectsMovement() {
    Serial.println("Motion Detected!!!");
}

String calculateTankCapacity(float distance) {
    if (distance >= 100) {
        return "full";
    } else if (distance >= 50 && distance < 100) {
        return "50%";
    } else if (distance >= 20 && distance < 50) {
        return "near empty";
    } else {
        return "empty";
    }
}

String parseSensorData(float temperature, float humidity, float distance, float gasLevel) {
    String tankStatus = calculateTankCapacity(distance);
    String data = "{\"temperature\":" + String(temperature, 2) + 
                  ", \"humidity\":" + String(humidity, 2) +
                  ", \"tank_capacity\":\"" + tankStatus +
                  "\", \"gas_level\":" + String(gasLevel) + "}";
    return data;
}

void handleSensorReadings() {
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    long duration = pulseIn(ECHO_PIN, HIGH);
    distance_cm = (duration / 2) / 29.09;
}

void onConnectionEstablished() {
}

void loop() {
    client.loop();
    handleSensorReadings();
    readSendData();
}

