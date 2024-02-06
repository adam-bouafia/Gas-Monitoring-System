import paho.mqtt.client as mqtt
import json

MQTT_BROKER = "test.mosquitto.org"
MQTT_PORT = 1883
ACTUATOR_COMMANDS_TOPIC = "univaq/actuatorCommands"

def on_connect(client, userdata, flags, rc):
    print("Connected with result code " + str(rc))
    client.subscribe(ACTUATOR_COMMANDS_TOPIC)

def on_message(client, userdata, msg):
    command = json.loads(msg.payload.decode())
    if command['command'] == "openValve":
        print("Opening gas valve.")
        
    elif command['command'] == "closeValve":
        print("Closing gas valve.")


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(MQTT_BROKER, MQTT_PORT, 60)
client.loop_forever()
