import logging
from queue import Queue, Empty

# Objects
from process_manager import ProcessManager, SystemState
from i2c_handler import i2c_handler
from gps_handler import gps_handler
from mqtt_handler import MqttHandler, MQTT_PUB_TOPICS
from utils.daq_controller_tester import DaqTester
from const import Sensor
from data_processor import (
    DataProcessor,
    GpsProcessor,
    ShockTravelProcessor,
    TireTemperatureProcessor,
)


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

    data_processor: dict[Sensor, DataProcessor] = {
        Sensor.GPS: GpsProcessor(controller.mqtt, MQTT_PUB_TOPICS["GPS_TOPIC"]),
        # Sensor.IMU: None,
        Sensor.TIRE_TEMPERATURE: TireTemperatureProcessor(controller.mqtt, MQTT_PUB_TOPICS["IR_TEMPERATURE_TOPIC"]),
        Sensor.SHOCK_TRAVEL: ShockTravelProcessor(controller.mqtt, MQTT_PUB_TOPICS["SHOCK_TRAVEL_TOPIC"]),
    }

    while controller.state != SystemState['SHUTDOWN']:
        try:
            qdata = controller.data_queue.get(block=True, timeout=1)
        except Empty:
            continue

        if qdata is None:
            continue

        assert len(qdata) == 3
        assert isinstance(qdata[0], Sensor)

        sensor, dt, data = qdata

        try:
            data_processor[sensor].process(dt, data)
        except KeyError:
            print(f"{sensor} does not have an associated DataProcessor")

    # Shutdown gracefully
    controller.shutdown()


if __name__ == '__main__':
    main()
