"""MQTT Tester

This script allows the user to connect the HiveMQ MQTT broker in the cloud
and send MQTT messages to control the Data Acquisition System running on
the Raspberry Pi
"""
import paho.mqtt.client as mqtt

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected successfully")
    else:
        print("Connect returned result code: " + str(rc))

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print("Received message: " + msg.topic + " -> " + msg.payload.decode("utf-8"))

def on_disconnect(client, userdata, rc):
    print("disconnected with result code "+str(rc))

# create the client
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.on_disconnect = on_disconnect
# enable TLS
client.tls_set(tls_version=mqtt.ssl.PROTOCOL_TLS)

# set username and password
client.username_pw_set("username", "password")

# connect to HiveMQ Cloud on port 8883
client.connect("78da1aca5bac48ceb4c9d7eff3de95e9.s1.eu.hivemq.cloud", 8883)

MQTT_TOPICS = {
    "SHUTDOWN_PIN_TOPIC": "commands/shutdown",
    "STATE_TOPIC": "commands/state"
}

SystemState = {
    '0': 'ACTIVE',
    '1': 'PAUSED',
    '2': 'SHUTDOWN'
}

# In an infinite loop, send MQTT messages based on user input
while True:
    user_prompt = """Choose topics:
        -0:Shutdown Pin
        -1:State\n"""
    topic_choice = input(user_prompt)

    if topic_choice == '0':
        topic = MQTT_TOPICS['SHUTDOWN_PIN_TOPIC']
        user_prompt = "Shutdown Pin options: 0:Low, 1:High\n"
        command = input(user_prompt)

    elif topic_choice == '1':
        topic = MQTT_TOPICS['STATE_TOPIC']
        user_prompt = "State options: 0:Active, 1:Paused, 2:Shutdown\n"
        command = SystemState[input(user_prompt)]

    else:
        print("Invalid topic choice")
        continue

    client.publish(topic, command)
