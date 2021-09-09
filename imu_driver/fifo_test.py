import time
import sys

import board
import busio
from adafruit_lsm6ds.lsm6ds33 import LSM6DS33
from adafruit_lsm6ds import Rate, AccelRange, GyroRange
import pigpio


# Accept I2C clock speed as an argument, default to 100 kHz
try:
    I2C_FREQUENCY = int(sys.argv[0])
except:
    I2C_FREQUENCY = 100000
INT_PIN = 14


def imu_fth_isr(gpio, level, tick):
    """Interrupt service routine for FIFO threshold interrupt

    Args:
        gpio: The GPIO that changed state
        level: State that the GPIO changed to. Low: 0, high: 1, no change: 2.
        tick: Number of microseconds since boot (max 2^32 - 1, then wraps back to 0)
    """
    isr_time = time.time()

    # Sometimes INT1 can trigger again as the FIFO is being read and filled
    # back up at the same time. If the time since the last tick is less than
    # 0.1s then exit the ISR.
    global last_tick
    MIN_TICK_DIFF_US = 10**5 
    tick_diff = pigpio.tickDiff(last_tick, tick)
    print(f"Time since last tick {tick_diff / 10**6} seconds")
    if tick_diff < MIN_TICK_DIFF_US:
        return

    global fifo_start
    print(f"Interrupt at {isr_time}")
    print(f"FIFO fill time: {isr_time - fifo_start:4.03f} seconds")
    fifo_start = isr_time

    # Read FIFO status
    status1 = imu._fifo_status1
    status2 = imu._fifo_status2
    status3 = imu._fifo_status3
    status4 = imu._fifo_status4

    # Number of unread words (16 bits) 
    unread_words = ((status2 & 0x0F) << 8) + status1
    print(f"Words in FIFO: {unread_words}")

    # Pattern index
    # In our case, the accelerometer and gyroscope data rates are equal, so the
    # pattern is in [0:5] where
    # 0 -> Gx
    # 1 -> Gy
    # 2 -> Gz
    # 3 -> Ax
    # 4 -> Ay
    # 5 -> Az
    pattern_index = (status4 << 8) + status3
    print(f"Index of next reading: {pattern_index}")

    # Read in multiples of 6, the number of readings from Gx to Az
    BYTES_PER_WORD = 2
    WORDS_PER_PATTERN = 6
    words_to_read = unread_words // WORDS_PER_PATTERN * WORDS_PER_PATTERN
    buffer_size = words_to_read * BYTES_PER_WORD
    buffer = bytearray(buffer_size)
    FIFO_DATA_OUT_L = bytearray(b'\x3E')

    # Read FIFO data into buffer
    start_time = time.time()
    imu.i2c_device.write_then_readinto(FIFO_DATA_OUT_L, buffer)
    end_time = time.time()
    total_read_time = end_time - start_time
    print(f"{buffer_size} bytes read in {total_read_time:.6f} seconds. {buffer_size/total_read_time:.0f} bytes/s")

    # Read FIFO status
    status1 = imu._fifo_status1
    status2 = imu._fifo_status2
    status3 = imu._fifo_status3
    status4 = imu._fifo_status4
    unread_words = ((status2 & 0x0F) << 8) + status1
    print(f"Words in FIFO: {unread_words}")
    pattern_index = (status4 << 8) + status3
    print(f"Index of next reading: {pattern_index}")

    last_tick = tick

    # Print data
    PREVIEW_BYTES = 12
    print(f"buffer = {buffer[:PREVIEW_BYTES].hex()} ... {buffer[-PREVIEW_BYTES:].hex()} | Len: {len(buffer)}")
    data = [parse_fifo_data(buffer[i:i+2]) for i in range(0, len(buffer), 2)]
    print(f"data = [{', '.join(map(str, data[:PREVIEW_BYTES]))}, ..., {', '.join(map(str, data[-PREVIEW_BYTES:]))}] | Len: {len(data)}")

    print()

def parse_fifo_data(data: bytearray) -> None:
    """Parse FIFO data from bytes to decimal."""
    return int.from_bytes(data, byteorder='little', signed=True)



# Initialize I2C bus
i2c = busio.I2C(board.SCL, board.SDA, frequency=I2C_FREQUENCY)

# Initialize LSM6DS33
# Resets the device and sets:
#   Accelerometer data rate = 104 Hz
#   Gyroscope data rate = 104 Hz
#   Accelerometer range = 4 g
#   Gyroscope range = 250 dps
#   BDU = 1 (output registers not updated until MSB and LSB have been read)
imu = LSM6DS33(i2c)

# # Set data rate for accelerometer and gyroscope
# imu.accelerometer_data_rate = Rate.RATE_12_5_HZ
# imu.gyro_data_rate = Rate.RATE_12_5_HZ


# FIFO Config
# Set FIFO watermark to 3072 (0x0C00) words (16 bits/word)
# 3072 samples will take up 6 kbyte, 3/4 of the FIFO
imu._fifo_threshold_l = 0x00
imu._fifo_threshold_h = 0x01

# No decimation for both gyroscope and accelerometer
imu._gyro_fifo_dec = 1
imu._accel_fifo_dec = 1
imu._timer_fifo_dec = 0

#imu._fifo_mode = 0
# Continuous mode. If the FIFO is full, the new sample overwrites the older one.
imu._fifo_mode = 6

# Set FIFO ODR to 12.5 Hz = 1
# Set FIFO ODR to 52 Hz = 3
# Set FIFO ODR to 104 Hz = 4
imu._fifo_data_rate = 3
fifo_start = time.time()

# Set INT1 to trigger when FIFO threshold is reached
imu._int1_full_set = True
imu._fifo_timestamp_en = True


# Connect to pigpiod
GPIO = pigpio.pi() 
if GPIO.connected:
    print("Connected to pigpiod")
else:
    exit(1)

# INT1 -> GPIO
GPIO.set_mode(INT_PIN, pigpio.INPUT)
GPIO.set_pull_up_down(INT_PIN, pigpio.PUD_DOWN)
callback_int = GPIO.callback(INT_PIN, pigpio.RISING_EDGE, imu_fth_isr)
last_tick = 0

try:
    while True:
        time.sleep(1)
finally:
    imu.reset()
    GPIO.stop()
