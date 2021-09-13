import paho.mqtt.client as mqtt
import json
import time
import RPi.GPIO as GPIO
import logging

from process_manager import SystemState

logger = logging.getLogger(__name__)
logger.setLevel(logging.DEBUG)

# Front-end communication topics
MQTT_PUB_TOPICS = {
    "GPS_TOPIC": "events/gps",
    "IMU_TOPIC": "events/imu",
    "SHOCK_TRAVEL_TOPIC": "events/shockTravel",
    "IR_TEMPERATURE_TOPIC": "events/irTemperature",
}

# Command topics
MQTT_SUB_TOPICS = {
    "SHUTDOWN_PIN_TOPIC": "commands/shutdown",
    "STATE_TOPIC": "commands/state"
}

# Shutdown circuit GPIO pin
SHUTDOWN_PIN = 18

class MqttHandler():
    """
    Mqtt Handler initializes MQTT client and connects to broker
    ...
    Attributes
    ----------
    client_id : string
        Name of the client
    broker_ip : string
        IP address of MQTT broker

    Methods
    -------
    __on_disconnect()
        Notify if client disconnects from broker
    __on_connect()
        Subscribe to MQTT topics on connection to broker
    __on_message()
        Handle incoming MQTT messages
    __setShutdownPin()
        Set output of shutdown pin
    """
    def __init__(self, client_id, broker_ip, controller):
        self.controller = controller

        GPIO.setup(SHUTDOWN_PIN, GPIO.OUT, initial=GPIO.LOW)

        self.client = mqtt.Client(client_id=client_id, clean_session=True) # Initialize client
        self.client.on_connect = self.__on_connect # Call this function when client successfully connects
        self.client.on_message = self.__on_message # Call this function when message is received on a subscribed topic
        self.client.on_disconnect = self.__on_disconnect

        # enable TLS
        self.client.tls_set(tls_version=mqtt.ssl.PROTOCOL_TLS)

        # set username and password
        self.client.username_pw_set("username", "password")
        self.client.connect(broker_ip, 8883, 10) # Connect to the local MQTT broker

        self.client.loop_start() # Start the MQTT Client

    def __on_disconnect(self, client, userdata, rc):
        """Called automatically to notify if client disconnects from broker
        Parameters
        ----------
        client : MQTT client
            MQTT client
        userdata : data
            Private user data, default=empty
        rc : int
            disconnection state
        """
        print("disconnected with result code "+str(rc))

    def __on_connect(self, client, userdata, flags, rc):
        """Called automatically when client connects to broker, subscribe to topics
        Parameters
        ----------
        client : MQTT client
            MQTT client
        userdata : data
            Private user data, default=empty
        flags : dict
            response flags sent by the broker
        rc : int
            connection state
        """
        print("Connected with result code "+str(rc))

        # Subscribing in on_connect() means that if we lose the connection and
        # reconnect then subscriptions will be renewed.
        for key in MQTT_SUB_TOPICS.keys():
            client.subscribe(MQTT_SUB_TOPICS[key])

    def __on_message(self, client, userdata, msg):
        """Called automatically when a message is received from the broker.
        Parameters
        ----------
        client : MQTT client
            MQTT client
        userdata : data
            Private user data, default=empty
        msg : MQTTMessage {topic, payload, qos, retain}
            Message received
        """

        try:
            print(msg.topic+" "+str(msg.payload))
            topic = msg.topic
            data = msg.payload.decode('utf-8')

            if topic == MQTT_SUB_TOPICS['SHUTDOWN_PIN_TOPIC']:
                self.__setShutdownPin(1)
            elif topic == MQTT_SUB_TOPICS['STATE_TOPIC']:
                self.__setSystemState(data)
            else:
                print("Invalid topic " + msg.topic)

        except Exception as e:
            print(e)


    def __setShutdownPin(self, pinValue):
        """Set the value of the shutdown pin.
        Parameters
        ----------
        pinValue : int
            GPIO output
        """
        print("!!!! SHUTDOWN INITIATED")
        if pinValue == 1:
            gpio_value = GPIO.HIGH
        elif pinValue == 0:
            gpio_value = GPIO.LOW
        else:
            print("Invalid Shutdown pin setting")
            return

        GPIO.output(SHUTDOWN_PIN, gpio_value)

    def __setSystemState(self, value):
        """Set the system state.
        Parameters
        ----------
        value : str
            Value from the SystemState dict.
        """
        print(value)
        if value == SystemState['SHUTDOWN']:
            self.controller.state = SystemState['SHUTDOWN']
        else:
            self.controller.set_system_state(value)

