#include <EspMQTTClient.h>
#include <Wire.h>
#include <DHTesp.h>
#include <AccelStepper.h>
#include <LiquidCrystal_I2C.h>

constexpr char MQTT_SERVER[] = "test.mosquitto.org";
constexpr char MQTT_CLIENT_ID[] = "mqtt-mack-pub-sub";
constexpr int MQTT_PORT = 1883;
constexpr char WIFI_SSID[] = "Wokwi-GUEST";
constexpr char WIFI_PASSWORD[] = "";

constexpr char SENSOR_DATA_TOPIC[] = "univaq/sensordata";
constexpr char ALERTS_TOPIC[] = "univaq/alerts";
constexpr char ACTUATOR_COMMANDS_TOPIC[] = "univaq/actuatorCommands";

constexpr int DHTPIN = 15;
constexpr int GAS_LEVER = 34;
constexpr int TRIG_PIN = 5;
constexpr int ECHO_PIN = 18;
constexpr int MOTION_SENSOR = 13;
constexpr int STATUS_LED_PIN = 2;
constexpr int FAN_RELAY_PIN = 16;
constexpr int VALVE_RELAY_PIN = 17;
constexpr int VALVE_FEEDBACK_PIN = 25;

constexpr int GAS_WARNING_THRESHOLD = 50;
constexpr int GAS_CRITICAL_THRESHOLD = 75;
constexpr int GAS_EMERGENCY_THRESHOLD = 90;

constexpr int LOW_TEMP_THRESHOLD = 10;
constexpr int HIGH_TEMP_THRESHOLD = 30;
constexpr int MEDIUM_TEMP_LOWER_THRESHOLD = 15;
constexpr int MEDIUM_TEMP_UPPER_THRESHOLD = 25;

constexpr int LOW_HUMIDITY_THRESHOLD = 20;
constexpr int HIGH_HUMIDITY_THRESHOLD = 80;
constexpr int MEDIUM_HUMIDITY_LOWER_THRESHOLD = 30;
constexpr int MEDIUM_HUMIDITY_UPPER_THRESHOLD = 60;

constexpr int LOW_CAPACITY_THRESHOLD = 25;
constexpr int VERY_LOW_CAPACITY_THRESHOLD = 10;
constexpr int MEDIUM_TANK_CAPACITY_LOWER_THRESHOLD = 40;
constexpr int MEDIUM_TANK_CAPACITY_UPPER_THRESHOLD = 60;

constexpr int GAS_LEVEL_CHANGE_THRESHOLD = 10;
constexpr unsigned long GAS_LEVEL_CHANGE_PERIOD = 10000;
constexpr int MEDIUM_GAS_LEVEL_LOWER_THRESHOLD = 20;
constexpr int MEDIUM_GAS_LEVEL_UPPER_THRESHOLD = 40;

constexpr int LCD_ADDR = 0x27;
constexpr int LCD_COLUMNS = 16;
constexpr int LCD_ROWS = 2;

DHTesp dhtsensor;
AccelStepper stepper(AccelStepper::DRIVER, 19, 2, 4, 23);
AccelStepper stepper1(AccelStepper::FULL4WIRE, 13, 12, 14, 27);
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLUMNS, LCD_ROWS);
EspMQTTClient client(WIFI_SSID, WIFI_PASSWORD, MQTT_SERVER, MQTT_CLIENT_ID, MQTT_PORT);

float duration_us, distance_cm;
unsigned long lastSendTime = 0;
const unsigned long sendInterval = 2000;
float lastGasLevel = 0;
unsigned long lastGasLevelTime = 0;

void detectsMovement();
bool isValveOpen();
void updateStatusLed(bool status);
void activateFan();
void deactivateFan();
void openGasValve();
void closeGasValveCommand();
void activateVentilation();
void closeGasValve();
void automateResponse(float gasLevel);
float calculateTankCapacity(float distance);
void handleEmergency(float gasLevel);
void checkAllConditions(float temperature, float humidity, float tankCapacity, float gasLevel);
void publishAlert(String alertType, String message, float value);
String parseSensorData(float temperature, float humidity, float distance, float gasLevel);
void handleSensorReadings();
void readSendData();
void onConnectionEstablished();


void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 collecting sensors data");
    stepper1.setMaxSpeed(8000);
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);
    pinMode(MOTION_SENSOR, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(MOTION_SENSOR), detectsMovement, RISING);
    
    dhtsensor.setup(DHTPIN, DHTesp::DHT22);
    pinMode(GAS_LEVER, INPUT);
    pinMode(FAN_RELAY_PIN, OUTPUT);
    pinMode(VALVE_RELAY_PIN, OUTPUT);
    pinMode(STATUS_LED_PIN, OUTPUT);
    pinMode(VALVE_FEEDBACK_PIN, INPUT_PULLUP); // Configure the feedback pin as input
    lcd.init(); // Initialize the LCD
    lcd.backlight();
}
bool isValveOpen() {
    return digitalRead(VALVE_FEEDBACK_PIN) == HIGH;
}
void detectsMovement() {
    Serial.println("Motion Detected!!!");
}
void updateStatusLed(bool status) {
    digitalWrite(STATUS_LED_PIN, status ? HIGH : LOW);
}
void activateFan() {
    digitalWrite(FAN_RELAY_PIN, HIGH);
    stepper1.setSpeed(1000);
    stepper1.runSpeed();
}
void deactivateFan() {
    digitalWrite(FAN_RELAY_PIN, LOW);
    stepper1.setSpeed(0);
    stepper1.runSpeed();

}
void openGasValve() {
    digitalWrite(VALVE_RELAY_PIN, HIGH);
}
void closeGasValveCommand() {
    digitalWrite(VALVE_RELAY_PIN, LOW);
}
void activateVentilation() {
    client.publish("univaq/actuatorCommands", "{\"command\":\"activateVentilation\"}");
    stepper1.setSpeed(3000);
    stepper1.runSpeed();
}
void closeGasValve() {
    client.publish("univaq/actuatorCommands", "{\"command\":\"closeGasValve\"}");
    stepper1.setSpeed(3000);
    stepper1.runSpeed();
}
void automateResponse(float gasLevel) {
    if (gasLevel > GAS_CRITICAL_THRESHOLD) {
        activateVentilation();
        if (gasLevel > GAS_EMERGENCY_THRESHOLD) {
            closeGasValve();
        }
    }
}

void handleEmergency(float gasLevel) {
    if (gasLevel > GAS_EMERGENCY_THRESHOLD) {
        closeGasValve();
        activateFan();
    }
}

float calculateTankCapacity(float distance) {
    if (distance >= 100) return 100.0;
    if (distance >= 50) return 50.0;
    if (distance >= 20) return 20.0;
    return 0.0;
}

void checkAllConditions(float temperature, float humidity, float tankCapacity, float gasLevel) {
    if (gasLevel > GAS_EMERGENCY_THRESHOLD) {
        publishAlert("Gas Emergency", "Critical gas level detected!", gasLevel);
    } else if (gasLevel > GAS_CRITICAL_THRESHOLD) {
        publishAlert("Gas Critical", "High gas level detected!", gasLevel);
    } else if (gasLevel > GAS_WARNING_THRESHOLD) {
        publishAlert("Gas Warning", "Elevated gas level detected!", gasLevel);
    } else if (gasLevel >= MEDIUM_GAS_LEVEL_LOWER_THRESHOLD && gasLevel <= MEDIUM_GAS_LEVEL_UPPER_THRESHOLD) {
        publishAlert("Medium Gas Level", "Gas level is within a normal range.", gasLevel);
    }

    if (temperature < LOW_TEMP_THRESHOLD) {
        publishAlert("Low Temperature", "Temperature too low!", temperature);
    } else if (temperature > HIGH_TEMP_THRESHOLD) {
        publishAlert("High Temperature", "Temperature too high!", temperature);
    } else if (temperature >= MEDIUM_TEMP_LOWER_THRESHOLD && temperature <= MEDIUM_TEMP_UPPER_THRESHOLD) {
        publishAlert("Medium Temperature", "Temperature is within a normal range.", temperature);
    }

    if (humidity < LOW_HUMIDITY_THRESHOLD) {
        publishAlert("Low Humidity", "Humidity too low!", humidity);
    } else if (humidity > HIGH_HUMIDITY_THRESHOLD) {
        publishAlert("High Humidity", "Humidity too high!", humidity);
    } else if (humidity >= MEDIUM_HUMIDITY_LOWER_THRESHOLD && humidity <= MEDIUM_HUMIDITY_UPPER_THRESHOLD) {
        publishAlert("Medium Humidity", "Humidity is within a normal range.", humidity);
    }

    if (tankCapacity < VERY_LOW_CAPACITY_THRESHOLD) {
        publishAlert("Very Low Tank Capacity", "Tank capacity critically low!", tankCapacity);
    } else if (tankCapacity < LOW_CAPACITY_THRESHOLD) {
        publishAlert("Low Tank Capacity", "Tank capacity low!", tankCapacity);
    } else if (tankCapacity >= MEDIUM_TANK_CAPACITY_LOWER_THRESHOLD && tankCapacity <= MEDIUM_TANK_CAPACITY_UPPER_THRESHOLD) {
        publishAlert("Medium Tank Capacity", "Tank capacity is within a normal range.", tankCapacity);
    }

    if (millis() - lastGasLevelTime > GAS_LEVEL_CHANGE_PERIOD) {
        if (abs(gasLevel - lastGasLevel) > GAS_LEVEL_CHANGE_THRESHOLD) {
            publishAlert("Rapid Gas Change", "Rapid change in gas level detected!", gasLevel);
        }
        lastGasLevel = gasLevel;
        lastGasLevelTime = millis();
    }
    automateResponse(gasLevel);
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
    distance_cm = (pulseIn(ECHO_PIN, HIGH, 20000) / 2) / 29.09; // Added timeout for pulseIn
}
void onConnectionEstablished() {
}

void loop() {
    client.loop();
    handleSensorReadings();
    readSendData();
    
    float temperature = dhtsensor.getTemperature();
    float humidity = dhtsensor.getHumidity();
    if (isnan(temperature) || isnan(humidity)) { 
        Serial.println("Failed to read from DHT sensor!");
        return;
    }
    float gasLevel = map(analogRead(GAS_LEVER), 0, 4095, 0, 100);
    float tankCapacity = calculateTankCapacity(distance_cm); 
    
    checkAllConditions(temperature, humidity, tankCapacity, gasLevel);
}