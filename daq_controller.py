from queue import Queue
from time import sleep
import json

# Database
from query import insertRecord

# Objects
from process_manager import ProcessManager, SystemState
from i2c_handler import i2c_handler
from gps_handler import gps_handler
from mqtt_handler import *
from utils.daq_controller_tester import *

# Temperature Calculation
from bisect import bisect_left

# Utility functions
from utils.utils import round_json

LOCATION = "Pi"
DASHBOARD_NAME = "Test"
MQTT_BROKER_IP = "78da1aca5bac48ceb4c9d7eff3de95e9.s1.eu.hivemq.cloud"
MQTT_PRECISION = 3

logging.basicConfig(level=logging.NOTSET)

logger = logging.getLogger(__name__)
logger.setLevel(logging.WARNING)

class daq_controller(object):
    def __init__(self, testmode=False):
        self.process_manager = ProcessManager(self)
        self.data_queue = Queue()
        self.state = SystemState['ACTIVE']

        if testmode == False:
            self._i2c = i2c_handler(self)
            self._gps = gps_handler(self)
        else:
            daq_tester = DaqTester(self)
            print("Test mode engaged")

        self.mqtt = MqttHandler(DASHBOARD_NAME, MQTT_BROKER_IP, self)

    def shutdown(self):
        logger.warning("SHUTTING DOWN...")
        self._i2c.shutdown()
        self._gps.shutdown()
        logger.warning("I2C has been shut down")
        self.mqtt.client.disconnect()
        exit()

    def set_system_state(self, state):
        if state == SystemState['PAUSED'] or state == SystemState['ACTIVE']:
            self._i2c.pause(state)
            self._gps.pause(state)
        else:
            logger.warning("Invalid system state set")



def main():
    controller = daq_controller(testmode=False)

    while controller.state != SystemState['SHUTDOWN']:
        if controller.data_queue.qsize() > 0:
            dt, sensor, data = controller.data_queue.get()
            if sensor == "gps":
                data['dt'] = dt.isoformat()
                value = json.dumps(data, separators=(',', ':'))
                controller.mqtt.client.publish(MQTT_PUB_TOPICS['GPS_TOPIC'],
                                  payload=round_json(value, MQTT_PRECISION),
                                  qos=2,
                                  retain=False)

            elif sensor == "imu":
                value = json.dumps(data, separators=(',', ':'))
                controller.mqtt.client.publish(MQTT_PUB_TOPICS['IMU_TOPIC'],
                                  payload=round_json(value, MQTT_PRECISION),
                                  qos=2,
                                  retain=False)

            elif sensor == "shock_travel":
                data = {
                    "shock_travel": list(map(shock_travel_convert, data)),
                    'dt': dt.isoformat(),
                }
                value = json.dumps(data, separators=(',', ':'))
                controller.mqtt.client.publish(MQTT_PUB_TOPICS['SHOCK_TRAVEL_TOPIC'],
                                  payload=round_json(value, MQTT_PRECISION),
                                  qos=2,
                                  retain=False)

            elif sensor == "ir_temperature":
                data = {
                    "temperatures": list(map(ir_temperature_convert, data)),
                    'dt': dt.isoformat(),
                }
                value = json.dumps(data, separators=(',', ':'))
                controller.mqtt.client.publish(MQTT_PUB_TOPICS['IR_TEMPERATURE_TOPIC'],
                                  payload=round_json(value, MQTT_PRECISION),
                                  qos=2,
                                  retain=False)

            insertRecord(dt, LOCATION, DASHBOARD_NAME, sensor, value)
            logger.info(f"{dt}, {LOCATION}, {DASHBOARD_NAME}, {sensor}, {value}")

        else:
            sleep(0.01)

    # Shutdown gracefully
    controller.shutdown()


TEMP_CORRECTION_FACTOR = 0.98
TEMP_VOLT_LOOKUP = [-1.35, -1.11, -0.86, -0.59, -0.31, 0.00, 0.32, 0.67, 1.03, 1.41, 1.81, 2.24, 2.68, 3.14, 3.62, 4.13, 4.66, 5.21, 5.78, 6.38, 7.00]
TEMP_LOOKUP = [0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50,55, 60, 65, 70, 75, 80, 85, 90, 95, 100]
def ir_temperature_convert(voltage):
    """Converts a voltage reading to a temperature.

    The minimum temperature is 0 and the maximum temperature is 100.

    voltage : the voltage of the sensor
    """
    v_ref = voltage - 1.01
    v_amp = v_ref / 101
    v_mv = v_amp * 1000
    v_corr = v_mv / TEMP_CORRECTION_FACTOR
    tcf = 1 + (30 - 25) * (-0.0045)
    v_offs = 0.32 * tcf
    ref = v_corr + v_offs
    tc = ref / tcf

    return lookup(tc, TEMP_VOLT_LOOKUP, TEMP_LOOKUP)


SHOCK_VOLT_LOOKUP = [0.003, 0.075, 0.345, 0.840, 1.413, 1.944, 2.505, 3.048, 3.618, 4.224, 4.827, 5.115, 5.241]
SHOCK_LOOKUP = [0, 0.5, 1, 2, 3, 4, 5, 6, 7, 8, 9, 9.5, 10]
def shock_travel_convert(voltage):
    """Converts a voltage reading to a shock travel distance.

    The minimum temperature is 0 and the maximum temperature is 100.

    voltage : the voltage of the sensor
    """
    return lookup(voltage, SHOCK_VOLT_LOOKUP, SHOCK_LOOKUP)

def lookup(voltage, volt_lookup, unit_lookup):
    # Out of range cases
    if voltage <= volt_lookup[0]:
        return unit_lookup[0]
    if voltage >= volt_lookup[-1]:
        return unit_lookup[-1]

    i = bisect_left(volt_lookup, voltage)
    # Linear interpolation
    k = (voltage - volt_lookup[i - 1]) / (volt_lookup[i] - volt_lookup[i - 1])
    return unit_lookup[i - 1] + ((unit_lookup[i] - unit_lookup[i - 1]) * k)

if __name__ == '__main__':
    main()
