import json
import logging
from threading import Lock, Thread
from time import sleep
from datetime import datetime, timezone

from const import Sensor

class DaqTester(object):
    """
    DAQ Testing object to simulate sensor data and add messages to the queue
    ...
    Attributes
    ----------
    controller : daq_controller object
        DAQ Controller

    Methods
    -------
    test_gps_thread()
        Simulate GPS data
    test_imu_thread()
        Simulate IMU data
    test_ir_thread()
        Simulate IR Temperature data
    test_shock_thread()
        Simulate Shock Travel data
    """
    def __init__(self, controller):
        self._controller = controller

        self.GPS_PERIOD = 1
        self.IMU_PERIOD = 0.1
        self.IR_PERIOD = 0.5
        self.SHOCK_PERIOD = 0.2

        self._test_gps_thread = Thread(target=self.test_gps_thread)
        self._test_imu_thread = Thread(target=self.test_imu_thread)
        self._test_ir_thread = Thread(target=self.test_ir_thread)
        self._test_shock_thread = Thread(target=self.test_shock_thread)

        self._test_gps_thread.start()
        self._test_imu_thread.start()
        self._test_ir_thread.start()
        self._test_shock_thread.start()

    def test_gps_thread(self):
        """
        Simulate GPS data
        """
        while True:
            dt = datetime.now(timezone.utc)
            gps_data = {
                "lat": 1,
                "lon": 2,
                "alt": 3,
                "speed": 4,
            }
            self._controller.data_queue.put((Sensor.GPS, dt, gps_data))
            sleep(self.GPS_PERIOD)

    def test_imu_thread(self):
        """
        Simulate IMU data
        """
        while True:
            dt = datetime.now(timezone.utc)
            a = [1, 2, 3]
            g = [4, 5 ,6]
            self._controller.data_queue.put((Sensor.IMU, dt, (a, g)))
            sleep(self.IMU_PERIOD)

    def test_ir_thread(self):
        """
        Simulate IR Temperature data
        """
        while True:
            voltage = [None] * 8
            dt = datetime.now(timezone.utc)
            for i in range(8):
                sleep(0.001)
                voltage[i] = 1+i/10
            self._controller.data_queue.put((Sensor.TIRE_TEMPERATURE, dt, voltage))
            sleep(self.IR_PERIOD)

    def test_shock_thread(self):
        """
        Simulate Shock Travel data
        """
        while True:
            voltage = [None] * 4
            dt = datetime.now(timezone.utc)
            for i in range(4):
                sleep(0.001)
                voltage[i] = i
            self._controller.data_queue.put((Sensor.SHOCK_TRAVEL, dt, voltage))
            sleep(self.SHOCK_PERIOD)

