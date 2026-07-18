#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>
#include <math.h>
#include <string.h>
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

    struct SteeringInput {
        const char *name;
        AdsDevice *adc;
        uint8_t channel;

        SteeringInput(const char *sensorName, AdsDevice *adcDevice, uint8_t adcChannel)
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

    AdsDevice adsU3("U3", ADCAddress::U3);
    SteeringInput steeringInput("SteeringAngleSensor", &adsU3, 2);

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

    SampleStats sampleChannel(SteeringInput &input) {
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

    float voltageToAngleDegrees(float volts) {
        if (!isfinite(volts) || STEERING_ANGLE_MAX_V <= STEERING_ANGLE_MIN_V) {
            return NAN;
        }

        float clampedVoltage = volts;
        if (clampedVoltage < STEERING_ANGLE_MIN_V) {
            clampedVoltage = STEERING_ANGLE_MIN_V;
        }
        else if (clampedVoltage > STEERING_ANGLE_MAX_V) {
            clampedVoltage = STEERING_ANGLE_MAX_V;
        }

        const float normalized = (clampedVoltage - STEERING_ANGLE_MIN_V) /
                                 (STEERING_ANGLE_MAX_V - STEERING_ANGLE_MIN_V);
        return normalized * STEERING_ANGLE_FULL_SCALE_DEG;
    }

    const char *readingStatus(AdsDevice &adc, const SampleStats &stats, float volts) {
        if (!adc.online) {
            return "ADC_OFFLINE";
        }

        if (stats.minRaw <= 0 || stats.maxRaw >= 32760) {
            return "SATURATED";
        }

        if (volts < STEERING_ANGLE_MIN_V) {
            return "BELOW_CALIBRATED_RANGE";
        }

        if (volts > STEERING_ANGLE_MAX_V) {
            return "ABOVE_CALIBRATED_RANGE";
        }

        if ((stats.maxRaw - stats.minRaw) <= 2) {
            return "UNCHANGED";
        }

        return "ADC_CHANNEL_READ";
    }

    const char *statusDescription(const char *status) {
        if (strcmp(status, "ADC_CHANNEL_READ") == 0) {
            return "valid ADC reading";
        }

        if (strcmp(status, "ADC_OFFLINE") == 0) {
            return "ADS1115 did not initialize";
        }

        if (strcmp(status, "SATURATED") == 0) {
            return "signal is at/near ADC limit";
        }

        if (strcmp(status, "BELOW_CALIBRATED_RANGE") == 0) {
            return "voltage below steering min";
        }

        if (strcmp(status, "ABOVE_CALIBRATED_RANGE") == 0) {
            return "voltage above steering max";
        }

        if (strcmp(status, "UNCHANGED") == 0) {
            return "raw value barely changed";
        }

        return "unknown status";
    }
}

void setup() {
    Serial.begin(BAUD_RATE);
    delay(1000);

    Serial.println("Steering angle sensor test");
    Serial.printf("I2C SDA=%d SCL=%d\n", I2C_SDA_PIN, I2C_SCL_PIN);
    Serial.printf(
        "Steering ADC: U3 addr=0x%02X channel 2, matching main firmware SteeringAngleSensor\n",
        static_cast<uint8_t>(ADCAddress::U3));
    Serial.printf(
        "Calibration: %.3f V to %.3f V maps to 0.0 to %.1f deg\n",
        STEERING_ANGLE_MIN_V,
        STEERING_ANGLE_MAX_V,
        STEERING_ANGLE_FULL_SCALE_DEG);
    Serial.println("Turn the steering sensor and look for raw/voltage/angle_deg changes.");

    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    initAds(adsU3);

    Serial.println();
    Serial.println("Reading format:");
    Serial.println("sensor | adc/address/channel | voltage | angle | raw latest/avg/min/max/span | status");
    Serial.println();
}

void loop() {
    const uint8_t address = static_cast<uint8_t>(steeringInput.adc->address);

    if (!steeringInput.adc->online) {
        Serial.printf(
            "%s | adc=%s address=0x%02X channel=%u | status=ADC_OFFLINE (%s)\n",
            steeringInput.name,
            steeringInput.adc->name,
            address,
            steeringInput.channel,
            statusDescription("ADC_OFFLINE"));
        delay(500);
        return;
    }

    SampleStats stats = sampleChannel(steeringInput);
    int16_t spanRaw = stats.maxRaw - stats.minRaw;
    float latestVolts = steeringInput.adc->ads.computeVolts(stats.latestRaw);
    float angleDeg = voltageToAngleDegrees(latestVolts);
    int32_t avgRaw = stats.count == 0 ? 0 : stats.sumRaw / stats.count;
    const char *status = readingStatus(*steeringInput.adc, stats, latestVolts);

    Serial.printf(
        "%s | adc=%s address=0x%02X channel=%u | voltage=%.4f V | angle=%.1f deg | raw latest=%d avg=%ld min=%d max=%d span=%d | status=%s (%s)\n",
        steeringInput.name,
        steeringInput.adc->name,
        address,
        steeringInput.channel,
        latestVolts,
        angleDeg,
        stats.latestRaw,
        static_cast<long>(avgRaw),
        stats.minRaw,
        stats.maxRaw,
        spanRaw,
        status,
        statusDescription(status));

    delay(500);
}
