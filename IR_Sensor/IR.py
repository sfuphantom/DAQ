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

print("{:>5}\t{:>5}\t{:>5}\t{:>5}".format("raw", "v", "Adjusted mV", "Temp"))

while True:

        # Subtracting the reference voltage (0.97V)
        chan.Vref = chan.voltage - 1.0
        # print("Vref:", chan.Vref)

        # Divide by the amplification (101)
        chan.Vamp = chan.Vref / 101
        # print("Vamp:", chan.Vamp)

        # Turn into mV (x1000)
        chan.Vmv = chan.Vamp * 1000
        # print("Vmv:", chan.Vmv)

        # Correcting the voltage
        Corr_Factor = 0.98
        chan.Vcorr = chan.Vmv / Corr_Factor

        # Auxillary step calculatiung
        chan.TCF = 1 + (30 - 25) * (-0.0045)
        chan.Voffs = 0.32 * chan.TCF
        chan.REF = chan.Vcorr + chan.Voffs
        chan.TC = chan.REF / chan.TCF

        from bisect import bisect_left

        def lookup(Fvolt, volts, temps):
                # Prints 0 deg for everything below -1.35mV
                if Fvolt <= volts[0]: return temps[0]
                # Prints 100 deg for everything above 7mV
                if Fvolt >= volts[-1]: return temps[-1]

                # Finding where the voltage fits in the list
                i = bisect_left(volts, Fvolt)

                # linear Interpolation
                # Y = Y1 + (Y2 - Y1) * (X - X1) / (X2 - X1)
                # k = (X - X1) / (X2 - X1)
                k = (Fvolt - volts[i-1]) / (volts[i] - volts[i - 1])
                # Y = Y1 + (Y2 - Y1) * k
                Ftemp = temps[i - 1] + ((temps[i] - temps[i - 1]) * k)

                return Ftemp

        volts = [-1.35, -1.11, -0.86, -0.59, -0.31, 0.00, 0.32,
        0.67, 1.03, 1.41, 1.81, 2.24, 2.68, 3.14,
        3.62, 4.13, 4.66, 5.21, 5.78, 6.38, 7.00]

        temps = [0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50,
        55, 60, 65, 70, 75, 80, 85, 90, 95, 100]

        Fvolt = chan.Vmv
        temps = lookup(Fvolt, volts, temps)

        print("CHAN 0: "+"{:>5}\t{:>5.3f}\t{:>5.3f}\t{:>5.3f}".format(chan.value, chan.voltage, chan.Vmv, temps))
        time.sleep(0.5)
