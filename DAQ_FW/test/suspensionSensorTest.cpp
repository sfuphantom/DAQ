#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include "systemConfig.h"

namespace {
    struct AdsDevice {
        const char *name;
        ADCAddress address;
        Adafruit_ADS1115 ads;
        bool i2cAck = false;
        bool online = false;

        AdsDevice(const char *deviceName, ADCAddress deviceAddress)
            : name(deviceName), address(deviceAddress) {}
    };

    struct SuspensionInput {
        const char *name;
        AdsDevice *adc;
        uint8_t channel;

        SuspensionInput(const char *sensorName, AdsDevice *adcDevice, uint8_t adcChannel)
            : name(sensorName), adc(adcDevice), channel(adcChannel) {}
    };

    struct SampleStats {
        int16_t latestRaw = 0;
        int16_t minRaw = 32767;
        int16_t maxRaw = -32768;
        int32_t sumRaw = 0;
        uint8_t count = 0;
    };

    constexpr uint8_t samplesPerChannel = 16;

    AdsDevice adsU2("U2", ADCAddress::U2);

    SuspensionInput suspensionInputs[] = {
        {"SuspensionSensor1", &adsU2, 0}, {"SuspensionSensor2", &adsU2, 1}, {"SuspensionSensor3", &adsU2, 2}, {"SuspensionSensor4", &adsU2, 3},
    };

    bool probeI2c(uint8_t address) {
        Wire.beginTransmission(address);
        return Wire.endTransmission() == 0;
    }

    void initAds(AdsDevice &device) {
        const uint8_t address = static_cast<uint8_t>(device.address);
        device.i2cAck = probeI2c(address);
        device.online = device.ads.begin(address);
        device.ads.setGain(GAIN_TWOTHIRDS);
        device.ads.setDataRate(RATE_ADS1115_860SPS);

        Serial.printf(
            "ADC %s addr=0x%02X i2c_ack=%s begin=%s\n",
            device.name,
            address,
            device.i2cAck ? "OK" : "FAIL",
            device.online ? "OK" : "FAIL");
    }

    SampleStats sampleChannel(SuspensionInput &input) {
        SampleStats stats;

        for (uint8_t i = 0; i < samplesPerChannel; ++i) {
            int16_t raw = input.adc->ads.readADC_SingleEnded(input.channel);
            stats.latestRaw = raw;
            stats.minRaw = min(stats.minRaw, raw);
            stats.maxRaw = max(stats.maxRaw, raw);
            stats.sumRaw += raw;
            ++stats.count;
        }

        return stats;
    }

    const char *readingStatus(AdsDevice &adc, const SampleStats &stats) {
        if (!adc.online) {
            return "ADC_OFFLINE";
        }

        if (stats.minRaw <= 0 || stats.maxRaw >= 32760) {
            return "SATURATED";
        }

        if ((stats.maxRaw - stats.minRaw) <= 2) {
            return "UNCHANGED";
        }

        return "ADC_CHANNEL_READ";
    }
}

void setup() {
    Serial.begin(BAUD_RATE);
    delay(1000);

    Serial.println("Suspension sensor test");
    Serial.printf("I2C SDA=%d SCL=%d\n", I2C_SDA_PIN, I2C_SCL_PIN);
    Serial.printf(
        "Suspension ADC: U2 addr=0x%02X, matching main firmware channels 0-3\n",
        static_cast<uint8_t>(ADCAddress::U2));
    Serial.println("Open channels can float; a printed voltage does not prove a sensor is attached.");
    Serial.println("Move the attached sensor and look for the channel whose raw/voltage value changes.");

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    initAds(adsU2);

    Serial.println("name,adc,address,channel,latest_raw,latest_volts,min_raw,max_raw,span_raw,avg_raw,status");
}

void loop() {
    for (SuspensionInput &input : suspensionInputs) {
        const uint8_t address = static_cast<uint8_t>(input.adc->address);

        if (!input.adc->online) {
            Serial.printf(
                "%s,%s,0x%02X,%u,,,ADC_OFFLINE\n",
                input.name,
                input.adc->name,
                address,
                input.channel);
            continue;
        }

        SampleStats stats = sampleChannel(input);
        int16_t spanRaw = stats.maxRaw - stats.minRaw;
        float latestVolts = input.adc->ads.computeVolts(stats.latestRaw);
        int32_t avgRaw = stats.count == 0 ? 0 : stats.sumRaw / stats.count;
        const char *status = readingStatus(*input.adc, stats);

        Serial.printf(
            "%s,%s,0x%02X,%u,%d,%.4f,%d,%d,%d,%ld,%s\n",
            input.name,
            input.adc->name,
            address,
            input.channel,
            stats.latestRaw,
            latestVolts,
            stats.minRaw,
            stats.maxRaw,
            spanRaw,
            static_cast<long>(avgRaw),
            status);
    }

    Serial.println();
    delay(500);
}
