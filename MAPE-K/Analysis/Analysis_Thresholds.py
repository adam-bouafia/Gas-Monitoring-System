import json
import paho.mqtt.client as mqtt
import random
import time

# MQTT Settings
MQTT_BROKER = "test.mosquitto.org"
MQTT_PORT = 1883
SENSOR_DATA_TOPIC = "univaq/sensordata"
ALERTS_TOPIC = "univaq/alerts"

# Thresholds
TEMPERATURE_HIGH_THRESHOLD = 30
TEMPERATURE_LOW_THRESHOLD = 10
TEMPERATURE_MEDIUM_LOWER_THRESHOLD = 15
TEMPERATURE_MEDIUM_UPPER_THRESHOLD = 25
HUMIDITY_HIGH_THRESHOLD = 80
HUMIDITY_LOW_THRESHOLD = 20
HUMIDITY_MEDIUM_LOWER_THRESHOLD = 30
HUMIDITY_MEDIUM_UPPER_THRESHOLD = 60
GAS_LEVEL_CRITICAL_THRESHOLD = 75
GAS_LEVEL_WARNING_THRESHOLD = 50
GAS_LEVEL_EMERGENCY_THRESHOLD = 90
GAS_LEVEL_CHANGE_THRESHOLD = 10
TANK_CAPACITY_VERY_LOW_THRESHOLD = 10
TANK_CAPACITY_LOW_THRESHOLD = 25
TANK_CAPACITY_MEDIUM_LOWER_THRESHOLD = 40
TANK_CAPACITY_MEDIUM_UPPER_THRESHOLD = 60

client = mqtt.Client()

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe(SENSOR_DATA_TOPIC)

def on_message(client, userdata, msg):
    sensor_data = json.loads(msg.payload)
    analyze_sensor_readings(sensor_data)

def analyze_sensor_readings(data):
    temperature = data['temperature']
    humidity = data['humidity']
    gas_level = data['gas_level']
    tank_capacity = data.get('tank_capacity', 0)
    alerts = []

    # Temperature Analysis
    if temperature >= TEMPERATURE_HIGH_THRESHOLD:
        alerts.append("High Temperature Alert")
    elif temperature <= TEMPERATURE_LOW_THRESHOLD:
        alerts.append("Low Temperature Alert")

    # Humidity Analysis
    if humidity >= HUMIDITY_HIGH_THRESHOLD:
        alerts.append("High Humidity Alert")
    elif humidity <= HUMIDITY_LOW_THRESHOLD:
        alerts.append("Low Humidity Alert")

    # Gas Level Analysis
    if gas_level >= GAS_LEVEL_EMERGENCY_THRESHOLD:
        alerts.append("Gas Level Emergency")
    elif gas_level >= GAS_LEVEL_CRITICAL_THRESHOLD:
        alerts.append("Critical Gas Level")
    elif gas_level >= GAS_LEVEL_WARNING_THRESHOLD:
        alerts.append("Gas Level Warning")

    # Analyze tank capacity alongside temperature, humidity, and gas level
    if tank_capacity <= TANK_CAPACITY_VERY_LOW_THRESHOLD:
        alerts.append("Tank Capacity Very Low Alert")
    elif tank_capacity <= TANK_CAPACITY_LOW_THRESHOLD:
        alerts.append("Tank Capacity Low Alert")
    elif TANK_CAPACITY_MEDIUM_LOWER_THRESHOLD <= tank_capacity <= TANK_CAPACITY_MEDIUM_UPPER_THRESHOLD:
        alerts.append("Tank Capacity Medium Alert")

    # Generate and publish alerts
    for alert in alerts:
        publish_alert(alert, data)

def publish_alert(alert, data):
    alert_message = {
        "alertType": alert, 
        "message": f"{alert} detected with value: {data.get('value', 'N/A')}",
        "value": data
    }
    client.publish(ALERTS_TOPIC, json.dumps(alert_message))
    print(f"Published Alert: {json.dumps(alert_message)}")

client.on_connect = on_connect
client.on_message = on_message

client.connect(MQTT_BROKER, MQTT_PORT, 60)

# Start the loop
client.loop_forever()
