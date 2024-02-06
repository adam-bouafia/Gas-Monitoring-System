import paho.mqtt.client as mqtt
import json
import time

# MQTT Broker settings
MQTT_BROKER = "test.mosquitto.org"
MQTT_PORT = 1883
VENTILATION_COMMAND_TOPIC = "univaq/actuatorCommands"

# This function gets called when the client connects to the broker
def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))

# Initialize MQTT Client
client = mqtt.Client()
client.on_connect = on_connect

client.connect(MQTT_BROKER, MQTT_PORT, 60)

def activate_ventilation():
    command = {"command": "activateVentilation"}
    client.publish(VENTILATION_COMMAND_TOPIC, json.dumps(command))
    print("Ventilation activated to mitigate high gas levels.")

# Main loop
try:
    client.loop_start()
    while True:
        
        activate_ventilation()
        time.sleep(10)  
except KeyboardInterrupt:
    client.disconnect()
    print("Disconnected from MQTT Broker.")
