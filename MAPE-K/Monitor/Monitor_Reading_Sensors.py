import paho.mqtt.client as mqtt
import time
import json
import random

# MQTT Broker settings
MQTT_BROKER = "test.mosquitto.org"
MQTT_PORT = 1883
MQTT_TOPIC = "univaq/sensordata"

# Simulate sensor data reading
def read_sensor_data():
    temperature = round(random.uniform(18, 30), 2)
    humidity = round(random.uniform(30, 70), 2)
    gas_level = round(random.uniform(0, 100), 2)
    tank_capacity = round(random.uniform(0, 100), 2)
    return temperature, humidity, gas_level, tank_capacity

# Callback when connected to MQTT broker
def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")

# Setup MQTT client and connect
client = mqtt.Client()
client.on_connect = on_connect
client.connect(MQTT_BROKER, MQTT_PORT, 60)

# Main loop to simulate sensor data reading and publishing
try:
    while True:
        temperature, humidity, gas_level, tank_capacity = read_sensor_data()
        data = {
            "temperature": temperature,
            "humidity": humidity,
            "gas_level": gas_level,
            "tank_capacity": tank_capacity
        }
        client.publish(MQTT_TOPIC, json.dumps(data))
        print(f"Published data: {data}")
        time.sleep(2)  # Simulate sensor reading interval
except KeyboardInterrupt:
    print("Simulation stopped by user")
