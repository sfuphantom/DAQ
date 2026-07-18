#include "flowPulse.h"
#include "sensorService.h"
#include "iadcSensor.h"
#include "systemConfig.h"
#include <Arduino.h>
#include <math.h>

/*
 This file configures the ADS1115 ADC devices and flow-pulse inputs used by the enabled sensors.
 It creates sensor instances based on compile-time configuration flags, initializes each sensor only
 when its required hardware is available, and collects readings into a shared SensorSnapshot.
 Sensor readings are grouped into critical cooling-system data and chassis data so they can be
 sampled independently at different rates.
*/

namespace{
    Adafruit_ADS1115 ADS_U1;
    Adafruit_ADS1115 ADS_U2;
    Adafruit_ADS1115 ADS_U3;
    Adafruit_ADS1115 ADS_U4;
    bool adsU1Online = false;
    bool adsU2Online = false;
    bool adsU3Online = false;
    bool adsU4Online = false;

    constexpr bool useAdsU1 = ENABLE_TEMP_SENSOR_1 ||
                              ENABLE_TEMP_SENSOR_2;
    constexpr bool useAdsU2 = ENABLE_SUSP_SENSOR_1 ||
                              ENABLE_SUSP_SENSOR_2 ||
                              ENABLE_SUSP_SENSOR_3 ||
                              ENABLE_SUSP_SENSOR_4;
    constexpr bool useAdsU3 = ENABLE_STEERING_ANGLE_SENSOR;
    constexpr bool useAdsU4 = false;
    constexpr bool useFlowPulse = ENABLE_FLOW_SENSOR_1 || ENABLE_FLOW_SENSOR_2;

    void configureAds(Adafruit_ADS1115 &ads){
        ads.setGain(GAIN_TWOTHIRDS);
        ads.setDataRate(RATE_ADS1115_860SPS);
    }

    bool initAdsIfUsed(bool shouldInit, Adafruit_ADS1115 &ads, ADCAddress address, const char *name){
        if (!shouldInit){
            return false;
        }

        bool online = ads.begin(static_cast<uint8_t>(address));
        if (online){
            configureAds(ads);
        }
        else{
            Logger::error("Failed to start ADC %s", name);
        }

        return online;
    }
}

#if ENABLE_TEMP_SENSOR_1
static CoolantTemperatureSensor CoolantTemperature1("CoolantTemperatureSensor", TEMP_SENSOR_1_CHANNEL, TEMP_SENSOR_1_ADC, &ADS_U1);
#endif
#if ENABLE_TEMP_SENSOR_2
static CoolantTemperatureSensor CoolantTemperature2("CoolantTemperatureSensor2", TEMP_SENSOR_2_CHANNEL, TEMP_SENSOR_2_ADC, &ADS_U1);
#endif
#if ENABLE_STEERING_ANGLE_SENSOR
static SteeringAngleSensor SteeringAngle("SteeringAngleSensor", STEERING_ANGLE_CHANNEL, STEERING_ANGLE_ADC, &ADS_U3);
#endif
#if ENABLE_SUSP_SENSOR_1
static SuspensionSensor Suspension1("SuspensionSensor1", SUSP_SENSOR_1_CHANNEL, SUSP_SENSOR_1_ADC, &ADS_U2);
#endif
#if ENABLE_SUSP_SENSOR_2
static SuspensionSensor Suspension2("SuspensionSensor2", SUSP_SENSOR_2_CHANNEL, SUSP_SENSOR_2_ADC, &ADS_U2);
#endif
#if ENABLE_SUSP_SENSOR_3
static SuspensionSensor Suspension3("SuspensionSensor3", SUSP_SENSOR_3_CHANNEL, SUSP_SENSOR_3_ADC, &ADS_U2);
#endif
#if ENABLE_SUSP_SENSOR_4
static SuspensionSensor Suspension4("SuspensionSensor4", SUSP_SENSOR_4_CHANNEL, SUSP_SENSOR_4_ADC, &ADS_U2);
#endif

void sensorServiceInit(){
    if (useFlowPulse){
        flowPulseInit();
    }

    adsU1Online = initAdsIfUsed(useAdsU1, ADS_U1, ADCAddress::U1, "U1");
    adsU2Online = initAdsIfUsed(useAdsU2, ADS_U2, ADCAddress::U2, "U2");
    adsU3Online = initAdsIfUsed(useAdsU3, ADS_U3, ADCAddress::U3, "U3");
    adsU4Online = initAdsIfUsed(useAdsU4, ADS_U4, ADCAddress::U4, "U4");

#if ENABLE_TEMP_SENSOR_1
    CoolantTemperature1.initialize(adsU1Online);
#endif
#if ENABLE_TEMP_SENSOR_2
    CoolantTemperature2.initialize(adsU1Online);
#endif
#if ENABLE_STEERING_ANGLE_SENSOR
    SteeringAngle.initialize(adsU3Online);
#endif
#if ENABLE_SUSP_SENSOR_1
    Suspension1.initialize(adsU2Online);
#endif
#if ENABLE_SUSP_SENSOR_2
    Suspension2.initialize(adsU2Online);
#endif
#if ENABLE_SUSP_SENSOR_3
    Suspension3.initialize(adsU2Online);
#endif
#if ENABLE_SUSP_SENSOR_4
    Suspension4.initialize(adsU2Online);
#endif
}

void sensorServiceReadCritical(SensorSnapshot &snapshot){
#if ENABLE_TEMP_SENSOR_1
    snapshot.temp1 = CoolantTemperature1.getData();
#else
    snapshot.temp1 = NAN;
#endif
#if ENABLE_TEMP_SENSOR_2
    snapshot.temp2 = CoolantTemperature2.getData();
#else
    snapshot.temp2 = NAN;
#endif
    if (useFlowPulse) {
        flowPulseUpdate();
    }
#if ENABLE_FLOW_SENSOR_1
    snapshot.flow1Lpm = flowPulseGetFlow1Lpm();
#else
    snapshot.flow1Lpm = NAN;
#endif
#if ENABLE_FLOW_SENSOR_2
    snapshot.flow2Lpm = flowPulseGetFlow2Lpm();
#else
    snapshot.flow2Lpm = NAN;
#endif
}

void sensorServiceReadChassis(SensorSnapshot &snapshot){
#if ENABLE_SUSP_SENSOR_1
    snapshot.susp1 = Suspension1.getData();
#else
    snapshot.susp1 = NAN;
#endif
#if ENABLE_SUSP_SENSOR_2
    snapshot.susp2 = Suspension2.getData();
#else
    snapshot.susp2 = NAN;
#endif
#if ENABLE_SUSP_SENSOR_3
    snapshot.susp3 = Suspension3.getData();
#else
    snapshot.susp3 = NAN;
#endif
#if ENABLE_SUSP_SENSOR_4
    snapshot.susp4 = Suspension4.getData();
#else
    snapshot.susp4 = NAN;
#endif
#if ENABLE_STEERING_ANGLE_SENSOR
    snapshot.steeringAngleDeg = SteeringAngle.getData();
#else
    snapshot.steeringAngleDeg = NAN;
#endif
}
