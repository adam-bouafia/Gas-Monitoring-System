import paho.mqtt.client as mqtt
import json
import time

# MQTT settings
MQTT_BROKER = "test.mosquitto.org"
MQTT_PORT = 1883
ACTUATOR_COMMANDS_TOPIC = "univaq/actuatorCommands"

# Initialize MQTT Client
client = mqtt.Client()

def on_connect(client, userdata, flags, rc):
    print(f"Connected with result code {rc}")

client.on_connect = on_connect

client.connect(MQTT_BROKER, MQTT_PORT, 60)

def send_actuator_command(command):
    client.publish(ACTUATOR_COMMANDS_TOPIC, json.dumps({"command": command}))
    print(f"Command '{command}' sent to actuator.")

# Main loop
try:
    client.loop_start()
    while True:
        
        send_actuator_command("closeGasValve")
        # Delay for demonstration purposes, replace with actual logic as needed
        time.sleep(10)
except KeyboardInterrupt:
    client.disconnect()
    print("Disconnected from MQTT Broker.")
