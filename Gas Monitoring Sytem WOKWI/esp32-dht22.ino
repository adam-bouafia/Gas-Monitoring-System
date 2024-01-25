#include "EspMQTTClient.h"
#include <Wire.h>
#include <DHTesp.h>
#include <AccelStepper.h>

EspMQTTClient client("Wokwi-GUEST", "", "test.mosquitto.org", "mqtt-mack-pub-sub", 1883);

// Sensor Pins
#define DHTPIN 15
#define GAS_LEVER 34
#define TRIG_PIN 5
#define ECHO_PIN 18
#define MOTION_SENSOR 13

// Gas Level Thresholds
#define GAS_WARNING_THRESHOLD 50   
#define GAS_CRITICAL_THRESHOLD 75  
#define GAS_EMERGENCY_THRESHOLD 90 

// Temperature Thresholds
#define LOW_TEMP_THRESHOLD 10   
#define HIGH_TEMP_THRESHOLD 30  
#define MEDIUM_TEMP_LOWER_THRESHOLD 15
#define MEDIUM_TEMP_UPPER_THRESHOLD 25

// Humidity Thresholds
#define LOW_HUMIDITY_THRESHOLD 20   
#define HIGH_HUMIDITY_THRESHOLD 80  
#define MEDIUM_HUMIDITY_LOWER_THRESHOLD 30
#define MEDIUM_HUMIDITY_UPPER_THRESHOLD 60

// Tank Capacity Thresholds
#define LOW_CAPACITY_THRESHOLD 25  
#define VERY_LOW_CAPACITY_THRESHOLD 10 
#define MEDIUM_TANK_CAPACITY_LOWER_THRESHOLD 40
#define MEDIUM_TANK_CAPACITY_UPPER_THRESHOLD 60

// Rate of Change for Gas Level
#define GAS_LEVEL_CHANGE_THRESHOLD 10 // Percentage change
#define GAS_LEVEL_CHANGE_PERIOD 10000  // Time in milliseconds (10 seconds)
#define MEDIUM_GAS_LEVEL_LOWER_THRESHOLD 20
#define MEDIUM_GAS_LEVEL_UPPER_THRESHOLD 40



DHTesp dhtsensor;
float duration_us, distance_cm;
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 2000;
float lastGasLevel = 0;
unsigned long lastGasLevelTime = 0;

AccelStepper stepper(AccelStepper::DRIVER, 19, 2, 4, 23);

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 collecting sensors data");

    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(MOTION_SENSOR, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(MOTION_SENSOR), detectsMovement, RISING);

    dhtsensor.setup(DHTPIN, DHTesp::DHT22);
    pinMode(GAS_LEVER, INPUT);
}

void detectsMovement() {
    Serial.println("Motion Detected!!!");
}

float calculateTankCapacity(float distance) {
    // Simplified tank capacity calculation
    if (distance >= 100) return 100.0;
    if (distance >= 50) return 50.0;
    if (distance >= 20) return 20.0;
    return 0.0;
}

void checkAllConditions(float temperature, float humidity, float tankCapacity, float gasLevel) {
    // Gas Level Alerts
    if (gasLevel > GAS_EMERGENCY_THRESHOLD) {
        publishAlert("Gas Emergency", "Critical gas level detected!", gasLevel);
    } else if (gasLevel > GAS_CRITICAL_THRESHOLD) {
        publishAlert("Gas Critical", "High gas level detected!", gasLevel);
    } else if (gasLevel > GAS_WARNING_THRESHOLD) {
        publishAlert("Gas Warning", "Elevated gas level detected!", gasLevel);
    } else if (gasLevel >= MEDIUM_GAS_LEVEL_LOWER_THRESHOLD && gasLevel <= MEDIUM_GAS_LEVEL_UPPER_THRESHOLD) {
        publishAlert("Medium Gas Level", "Gas level is within a normal range.", gasLevel);
    }

    // Temperature Alerts
    if (temperature < LOW_TEMP_THRESHOLD) {
        publishAlert("Low Temperature", "Temperature too low!", temperature);
    } else if (temperature > HIGH_TEMP_THRESHOLD) {
        publishAlert("High Temperature", "Temperature too high!", temperature);
    } else if (temperature >= MEDIUM_TEMP_LOWER_THRESHOLD && temperature <= MEDIUM_TEMP_UPPER_THRESHOLD) {
        publishAlert("Medium Temperature", "Temperature is within a normal range.", temperature);
    }

    // Humidity Alerts
    if (humidity < LOW_HUMIDITY_THRESHOLD) {
        publishAlert("Low Humidity", "Humidity too low!", humidity);
    } else if (humidity > HIGH_HUMIDITY_THRESHOLD) {
        publishAlert("High Humidity", "Humidity too high!", humidity);
    } else if (humidity >= MEDIUM_HUMIDITY_LOWER_THRESHOLD && humidity <= MEDIUM_HUMIDITY_UPPER_THRESHOLD) {
        publishAlert("Medium Humidity", "Humidity is within a normal range.", humidity);
    }

    // Tank Capacity Alerts
    if (tankCapacity < VERY_LOW_CAPACITY_THRESHOLD) {
        publishAlert("Very Low Tank Capacity", "Tank capacity critically low!", tankCapacity);
    } else if (tankCapacity < LOW_CAPACITY_THRESHOLD) {
        publishAlert("Low Tank Capacity", "Tank capacity low!", tankCapacity);
    } else if (tankCapacity >= MEDIUM_TANK_CAPACITY_LOWER_THRESHOLD && tankCapacity <= MEDIUM_TANK_CAPACITY_UPPER_THRESHOLD) {
        publishAlert("Medium Tank Capacity", "Tank capacity is within a normal range.", tankCapacity);
    }

    // Rapid Change in Gas Level
    if (millis() - lastGasLevelTime > GAS_LEVEL_CHANGE_PERIOD) {
        if (abs(gasLevel - lastGasLevel) > GAS_LEVEL_CHANGE_THRESHOLD) {
            publishAlert("Rapid Gas Change", "Rapid change in gas level detected!", gasLevel);
        }
        lastGasLevel = gasLevel;
        lastGasLevelTime = millis();
    }
}

void publishAlert(String alertType, String message, float value) {
    String alertMessage = "{\"alertType\":\"" + alertType + 
                          "\", \"message\": \"" + message + 
                          "\", \"value\":" + String(value, 2) + "}";
    client.publish("univaq/alerts", alertMessage);
}

void readSendData() {
    unsigned long currentTime = millis();
    if (currentTime - lastSendTime >= sendInterval) {
        TempAndHumidity data = dhtsensor.getTempAndHumidity();
        float gasLevel = map(analogRead(GAS_LEVER), 0, 4095, 0, 100);  
        String jsonData = parseSensorData(data.temperature, data.humidity, distance_cm, gasLevel);
        client.publish("univaq/sensordata", jsonData);
        Serial.println(jsonData);
        lastSendTime = currentTime;
    }
}

String parseSensorData(float temperature, float humidity, float distance, float gasLevel) {
    String tankStatus = String(calculateTankCapacity(distance));
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
    distance_cm = (pulseIn(ECHO_PIN, HIGH) / 2) / 29.09;
}

void onConnectionEstablished() {
}

void loop() {
    client.loop();
    handleSensorReadings();
    readSendData();
    
    float temperature = dhtsensor.getTemperature();
    float humidity = dhtsensor.getHumidity();
    float gasLevel = map(analogRead(GAS_LEVER), 0, 4095, 0, 100);
    float tankCapacity = calculateTankCapacity(distance_cm); 
    
    checkAllConditions(temperature, humidity, tankCapacity, gasLevel);
}