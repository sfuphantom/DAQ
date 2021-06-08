# SPDX-FileCopyrightText: 2021 ladyada for Adafruit Industries
# SPDX-License-Identifier: MIT

import time
import board
import busio
import adafruit_ads1x15.ads1115 as ADS
from adafruit_ads1x15.analog_in import AnalogIn

# Create the I2C bus
i2c = busio.I2C(board.SCL, board.SDA)

# Create the ADC object using the I2C bus
ads = ADS.ADS1115(i2c)

# Create single-ended input on channel 0
chan = AnalogIn(ads, ADS.P0)

# Create differential input between channel 0 and 1
# chan = AnalogIn(ads, ADS.P0, ADS.P1)

print("{:>5}\t{:>5}\t{:>5}".format("raw", "v", "distance"))

while True:
        ads.gain = 2/3
        from bisect import bisect_left

        def lookup(Fvolt, volts, distances):
                # Prints 0 deg for everything below -1.35mV
                if Fvolt <= volts[0]: return distances[0]
                # Prints 100 deg for everything above 7mV
                if Fvolt >= volts[-1]: return distances[-1]

                # Finding where the voltage fits in the list
                i = bisect_left(volts, Fvolt)

                # linear Interpolation
                # Y = Y1 + (Y2 - Y1) * (X - X1) / (X2 - X1)
                # k = (X - X1) / (X2 - X1)
                k = (Fvolt - volts[i-1]) / (volts[i] - volts[i - 1])
                # Y = Y1 + (Y2 - Y1) * k
                Fdistance = distances[i - 1] + ((distances[i] - distances[i - 1]) * k)

                return Fdistance

        volts = [0.003, 0.075, 0.345, 0.840, 1.413,
                1.944, 2.505, 3.048, 3.618,
                4.224, 4.827, 5.115, 5.241]

        distances = [0, 0.5, 1, 2, 3, 4, 5,
                     6, 7, 8, 9, 9.5, 10]

        Fvolt = chan.voltage
        distance = lookup(Fvolt, volts, distances)

        print("CHAN 0: "+"{:>5}\t{:>5.3f}\t{:>5.3f}".format(chan.value, chan.voltage, distance))
        time.sleep(0.5)
