import json
import logging
from threading import Lock, Thread
from time import sleep
from datetime import datetime

# Hardware Libraries
import board
import busio
import adafruit_ads1x15.ads1115 as ADS
from adafruit_ads1x15.analog_in import AnalogIn
from adafruit_lsm6ds.lsm6ds33 import LSM6DS33
from adafruit_lsm6ds import Rate, AccelRange, GyroRange
import RPi.GPIO as GPIO


class i2c_handler(object):
    ADC_ADDRESS = 0x48
    IMU_ADDRESS = 0x6a
    IR_TEMPERATURE_SELECTOR_PINS = [17, 27, 22]  # GPIO pin numbering
    SHOCK_TRAVEL_SELECTOR_PINS = [23, 24]  # GPIO pin numbering

    def __init__(self, controller):
        self._controller = controller
        self._i2c = busio.I2C(board.SCL, board.SDA)
        self.adc_connected = False
        self.imu_connected = False
        while not self._i2c.try_lock():
            pass
        try:
            i2c_addresses = self._i2c.scan()
            if self.ADC_ADDRESS in i2c_addresses:
                self.adc_connected = True
                print("ADC is connected with address %s" % hex(self.ADC_ADDRESS))
            else:
                print("ADC is not connected")
            if self.IMU_ADDRESS in i2c_addresses:
                self.imu_connected = True
                print("IMU is connected with address %s" % hex(self.IMU_ADDRESS))
            else:
                print("IMU is not connected")
        finally:
            self._i2c.unlock()

        if self.adc_connected:
            self._init_adc()
        if self.imu_connected:
            self._init_imu()

        GPIO.setmode(GPIO.BCM)
        GPIO.setup(
            self.IR_TEMPERATURE_SELECTOR_PINS + self.SHOCK_TRAVEL_SELECTOR_PINS,
            GPIO.OUT,
            initial=GPIO.LOW,
        )
        self._i2c_thread = Thread(target=self.i2c_thread)
        self._i2c_thread.start()

    def _init_adc(self):
        self._adc = ADS.ADS1115(self._i2c)
        self._adc_chan0 = AnalogIn(self._adc, ADS.P0)
        self._adc_chan2 = AnalogIn(self._adc, ADS.P2)

    def _init_imu(self):
        self._imu = LSM6DS33(self._i2c)
        # self._imu.accelerometer_data_rate = Rate.RATE_12_5_HZ
        # self._imu.accelerometer_range = AccelRange.RANGE_4G
        # self._imu.gyro_data_rate = Rate.RATE_104_HZ
        # self._imu.gyro_range = GyroRange.RANGE_250_DPS

    def read_imu(self):
        dt = datetime.utcnow()
        a = self._imu.acceleration
        g = self._imu.gyro
        self._controller.data_queue.put((dt, "imu", (a, g)))

    def read_ir_temperature(self):
        self._adc.gain = 1 # set ADC gain to 4.096 V
        sleep(0.001)
        voltage = [None] * 8
        dt = datetime.utcnow()
        for i in range(8):
            GPIO.output(
                self.IR_TEMPERATURE_SELECTOR_PINS,
                tuple([int(x) for x in list("{0:03b}".format(i))]),
            )
            sleep(0.001)
            voltage[i] = self._adc_chan0.voltage
        self._controller.data_queue.put((dt, "ir_temperature", voltage))

    def read_shock_travel(self):
        self._adc.gain = 2/3 # set ADC gain to 6.144 V
        sleep(0.001)
        voltage = [None] * 4
        dt = datetime.utcnow()
        for i in range(4):
            GPIO.output(
                self.SHOCK_TRAVEL_SELECTOR_PINS,
                tuple([int(x) for x in list("{0:02b}".format(i))]),
            )
            sleep(0.001)
            voltage[i] = self._adc_chan2.voltage
        self._controller.data_queue.put((dt, "shock_travel", voltage))

    def i2c_thread(self):
        while True:
            if self.adc_connected:
                self.read_ir_temperature()
            for i in range(50):
                if self.imu_connected:
                    self.read_imu()
                if self.adc_connected:
                    self.read_shock_travel()
                sleep(0.02)
