import logging
import numpy as np
from datetime import datetime, timedelta, timezone

# Hardware Libraries
from imu_driver.adafruit_lsm6ds.lsm6ds33 import LSM6DS33
from imu_driver.adafruit_lsm6ds import Rate, AccelRange, GyroRange
import RPi.GPIO as GPIO

logger = logging.getLogger(__name__)  
logger.setLevel(logging.DEBUG)

def parse_fifo_data(data: bytearray, signed: bool=True) -> None:
    """Parse FIFO data from bytes to decimal."""
    return int.from_bytes(data, byteorder='little', signed=signed)

class ImuController():
    """
    IMU Controller connects to IMU and handles reading the FIFO
    ...
    Attributes
    ----------
    i2c_handler : object
        I2C handler

    Methods
    -------
    _init_imu(fifo_theshold)
        Initialize IMU with FIFO configuration
    _imu_interrupt_handler()
        Handle FIFO threshold interrupt, process FIFO data
    _process_fifo_data(data_buffer)
        Process byte array into sensor readings
    """
    IMU_ADDRESS = 0x6a
    IMU_INT1_PIN = 14
    WORDS_PER_PATTERN = 18
    TIMESTAMP_INCREMENT = 25 # microseconds
    TIMESTAMP_RESET_THRESHOLD = 1000000
    FIFO_THRESHOLD = 180
    def __init__(self, i2c_handler):
        self._i2c_handler = i2c_handler
        self._init_imu(self.FIFO_THRESHOLD)

        # Setup INT1 Pin
        GPIO.setup(self.IMU_INT1_PIN, GPIO.IN)
        GPIO.add_event_detect(self.IMU_INT1_PIN, GPIO.RISING, callback=self._imu_interrupt_handler) 

    def _init_imu(self, fifo_threshold: int = 180):
        """Initialize IMU with FIFO configuration
        
        Parameters
        ----------
        fifo_threshold : int
            FIFO watermark theshold
        """
        self._imu = LSM6DS33(self._i2c_handler._i2c)
        # FIFO Config
        # Enable timestamp as 3rd data set in FIFO
        # Set resolution of timestamp
        # False: 6.4ms per bit, True: 25us per bit
        self._imu._timestamp_enable = True
        self._imu._fifo_timestamp_en = True
        self._imu._timestamp_resolution = True
        self._imu._timestamp_reg2 = 0xAA # Reset timestamp counter to 0


        # Set FIFO watermark to max 4096
        self._imu._fifo_threshold_l = fifo_threshold & 0x00FF
        self._imu._fifo_threshold_h = (fifo_threshold & 0x0F00) >> 8


        # No decimation for gyroscope and accelerometer and timestamp
        self._imu._gyro_fifo_dec = 1
        self._imu._accel_fifo_dec = 1
        self._imu._timer_fifo_dec = 1

        # Set FIFO ODR to 12.5 Hz = 1
        # Set FIFO ODR to 52 Hz = 3
        # Set FIFO ODR to 104 Hz = 4
        self._imu._fifo_data_rate = 3

        # Set INT1 to trigger when FIFO threshold is reached
        self._imu._int1_full_set = True

        # Continuous mode. If the FIFO is full, the new sample overwrites the older one.
        self._imu._fifo_mode = 6

    def _imu_interrupt_handler(self, channel):
        """Handle FIFO threshold interrupt, process FIFO data
        
        Parameters
        ----------
        channel : int
            GPIO channel interrupt occurred on
        """    
        # Read FIFO status
        status1 = self._imu._fifo_status1
        status2 = self._imu._fifo_status2
        status3 = self._imu._fifo_status3
        status4 = self._imu._fifo_status4

        # Number of unread words (16 bits) 
        unread_words = ((status2 & 0x0F) << 8) + status1

        logger.debug(f"Words in FIFO: {unread_words}")

        # Pattern index
        # In our case, the accelerometer and gyroscope data rates are equal, so the
        # pattern is in [0:5] where
        # 0 -> Gx   3 -> Ax
        # 1 -> Gy   4 -> Ay
        # 2 -> Gz   5 -> Az
        # 6-12 -> Timestamp and step counte
        # Read in multiples of 18, the number of readings from Gx to Timestamp
        BYTES_PER_WORD = 2
        words_to_read = unread_words // self.WORDS_PER_PATTERN * self.WORDS_PER_PATTERN
        buffer_size = words_to_read * BYTES_PER_WORD
        data_buffer = bytearray(buffer_size)
        FIFO_DATA_OUT_L = bytearray(b'\x3E')

        # Read FIFO data into buffer
        self._imu.i2c_device.write_then_readinto(FIFO_DATA_OUT_L, data_buffer)
        latest_ts = datetime.now(timezone.utc)

        gyroscope, acceleration, timestamp = self._process_fifo_data(data_buffer)

        for i in range(len(gyroscope)-1, -1, -1):
            # If timer has been reset, timestamps start from 0 again
            if timestamp[0] > timestamp[i]:
                ts =  latest_ts - timedelta(microseconds=(timestamp[i])*self.TIMESTAMP_INCREMENT)
            else:
                ts =  latest_ts - timedelta(microseconds=(timestamp[i]-timestamp[0])*self.TIMESTAMP_INCREMENT)

            processed_data = {
                "ax": acceleration[i,0],
                "ay": acceleration[i,1],
                "az": acceleration[i,2],
                "gx": gyroscope[i,0],
                "gy": gyroscope[i,1],
                "gz": gyroscope[i,2], 
            }   

            self._i2c_handler._controller.data_queue.put((ts, "imu", processed_data))

        logger.debug(f"Latest IMU Data: {latest_ts}, {processed_data}")

        if timestamp[-1] > self.TIMESTAMP_RESET_THRESHOLD:
            self._imu._timestamp_reg2 = 0xAA # Reset timestamp counter to 0

        
    def _process_fifo_data(self, data_buffer: bytearray):
        """Process byte array into sensor readings
        
        Parameters
        ----------
        data_buffer : bytearray
            FIFO bytearray

        Returns
        -------
        gyroscope : NumPy Array
            Array where each row is one xyz gyroscope reading
        acceleration : NumPy Array
            Array where each row is one xyz acceleration reading
        timestamp : list
            list of timestamp counter values
        """
        gyroscope = []
        acceleration = []
        timestamp = []

        # Convert bytes to sensor readings
        # Byte 0-5: Gx, Gy, Gz, Byte 6-11: Ax, Ay, Az
        # Byte 15: Timestamp[0:7], Byte 12: Timestamp[8:15], Byte 13: Timestamp[16:23]
        for i in range(0, len(data_buffer), self.WORDS_PER_PATTERN):
            gyroscope.append([parse_fifo_data(data_buffer[i+j:i+j+2]) for j in range(0, 6, 2)])
            acceleration.append([parse_fifo_data(data_buffer[i+j:i+j+2]) for j in range(6, 12, 2)])
            timestamp.append(parse_fifo_data([data_buffer[i+15], data_buffer[i+12], data_buffer[i+13]], signed=False))

        # Scale accelerometer and gyroscope values
        acceleration = self._imu._scale_xl_data(np.array(acceleration))
        gyroscope = self._imu._scale_gyro_data(np.array(acceleration))

        return gyroscope, acceleration, timestamp

