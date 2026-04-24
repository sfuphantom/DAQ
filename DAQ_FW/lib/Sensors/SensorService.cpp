#include "FlowPulse.h"
#include "SensorService.h"
#include "IADCSensor.h"
#include "system_config.h"
#include <Arduino.h>
#include <math.h>

namespace
{
    Adafruit_ADS1115 ADS_U1;
    Adafruit_ADS1115 ADS_U2;
    Adafruit_ADS1115 ADS_U3;
    Adafruit_ADS1115 ADS_U4;
    bool adsU1Online = false;
    bool adsU2Online = false;
    bool adsU3Online = false;
    bool adsU4Online = false;
}

#if ENABLE_TEMP_SENSOR_1
static CoolantTemperatureSensor CoolantTemperature1("CoolantTemperatureSensor", 0, ADCAddress::U2, &ADS_U2);
#endif
#if ENABLE_TEMP_SENSOR_2
static CoolantTemperatureSensor CoolantTemperature2("CoolantTemperatureSensor2", 1, ADCAddress::U2, &ADS_U2);
#endif
#if ENABLE_STEERING_ANGLE_SENSOR
static SteeringAngleSensor SteeringAngle("SteeringAngleSensor", 3, ADCAddress::U2, &ADS_U2);
#endif
#if ENABLE_SUSP_SENSOR_1
static SuspensionSensor Suspension1("SuspensionSensor1", 0, ADCAddress::U1, &ADS_U1);
#endif
#if ENABLE_SUSP_SENSOR_2
static SuspensionSensor Suspension2("SuspensionSensor2", 1, ADCAddress::U1, &ADS_U4);
#endif
#if ENABLE_SUSP_SENSOR_3
static SuspensionSensor Suspension3("SuspensionSensor3", 2, ADCAddress::U1, &ADS_U3);
#endif
#if ENABLE_SUSP_SENSOR_4
static SuspensionSensor Suspension4("SuspensionSensor4", 3, ADCAddress::U1, &ADS_U2);
#endif

void SensorService_Init()
{
    FlowPulse_Init();
    adsU1Online = ADS_U1.begin(static_cast<uint8_t>(ADCAddress::U1));
    adsU2Online = ADS_U2.begin(static_cast<uint8_t>(ADCAddress::U2));
    adsU3Online = ADS_U3.begin(static_cast<uint8_t>(ADCAddress::U3));
    adsU4Online = ADS_U4.begin(static_cast<uint8_t>(ADCAddress::U4));

    if (!adsU1Online)
    {
        Logger::Error("Failed to start ADC U1");
    }
    if (!adsU2Online)
    {
        Logger::Error("Failed to start ADC U2");
    }
    if (!adsU3Online)
    {
        Logger::Error("Failed to start ADC U3");
    }
    if (!adsU4Online)
    {
        Logger::Error("Failed to start ADC U4");
    }
#if ENABLE_TEMP_SENSOR_1
    CoolantTemperature1.Initialize(adsU2Online);
#endif
#if ENABLE_TEMP_SENSOR_2
    CoolantTemperature2.Initialize(adsU2Online);
#endif
#if ENABLE_STEERING_ANGLE_SENSOR
    SteeringAngle.Initialize(adsU2Online);
#endif
#if ENABLE_SUSP_SENSOR_1
    Suspension1.Initialize(adsU1Online);
#endif
#if ENABLE_SUSP_SENSOR_2
    Suspension2.Initialize(adsU1Online);
#endif
#if ENABLE_SUSP_SENSOR_3
    Suspension3.Initialize(adsU1Online);
#endif
#if ENABLE_SUSP_SENSOR_4
    Suspension4.Initialize(adsU1Online);
#endif
}

void SensorService_ReadCritical(SensorSnapshot &snapshot)
{
#if ENABLE_TEMP_SENSOR_1
    snapshot.temp1 = CoolantTemperature1.GetData();
#else
    snapshot.temp1 = NAN;
#endif
#if ENABLE_TEMP_SENSOR_2
    snapshot.temp2 = CoolantTemperature2.GetData();
#else
    snapshot.temp2 = NAN;
#endif
    FlowPulse_Update();
#if ENABLE_FLOW_SENSOR_1
    snapshot.flow1Lpm = FlowPulse_GetFlow1Lpm();
#else
    snapshot.flow1Lpm = NAN;
#endif
#if ENABLE_FLOW_SENSOR_2
    snapshot.flow2Lpm = FlowPulse_GetFlow2Lpm();
#else
    snapshot.flow2Lpm = NAN;
#endif
}

void SensorService_ReadChassis(SensorSnapshot &snapshot)
{
#if ENABLE_SUSP_SENSOR_1
    snapshot.susp1 = Suspension1.GetData();
#else
    snapshot.susp1 = NAN;
#endif
#if ENABLE_SUSP_SENSOR_2
    snapshot.susp2 = Suspension2.GetData();
#else
    snapshot.susp2 = NAN;
#endif
#if ENABLE_SUSP_SENSOR_3
    snapshot.susp3 = Suspension3.GetData();
#else
    snapshot.susp3 = NAN;
#endif
#if ENABLE_SUSP_SENSOR_4
    snapshot.susp4 = Suspension4.GetData();
#else
    snapshot.susp4 = NAN;
#endif
#if ENABLE_STEERING_ANGLE_SENSOR
    snapshot.steeringAngleDeg = SteeringAngle.GetData();
#else
    snapshot.steeringAngleDeg = NAN;
#endif
}
