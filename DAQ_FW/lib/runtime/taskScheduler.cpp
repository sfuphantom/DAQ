#include "TaskScheduler.h"
#include "FaultService.h"
#include "LoggingService.h"
#include "SDLoggingService.h"
#include "SensorService.h"
#include "SnapshotService.h"
#include "SimulatedSensors.h"
#include "TelemetryService.h"
#include "can.h"
#include "system_config.h"
#include "wheelSpeed.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <math.h>
#include <esp_task_wdt.h>

static void CriticalSensorsTask(void *parameter)
{
    (void)parameter;
    const TickType_t delayTicks = pdMS_TO_TICKS(20); // 50 Hz
    #if WATCHDOG_ENABLED
    esp_task_wdt_add(nullptr);
    #endif

    for (;;)
    {
        #if WATCHDOG_ENABLED
        esp_task_wdt_reset();
        #endif
        if (!SENSORS_ENABLED)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        SensorSnapshot snapshot = {};
        snapshot.timestampMs = millis();
        SensorService_ReadCritical(snapshot);
        snapshot.faultActive = FaultService_Update(snapshot.flow1Lpm, snapshot.flow2Lpm, snapshot.temp1, snapshot.temp2, CAN_ENABLED);

        SnapshotService_UpdateCritical(
            snapshot.temp1,
            snapshot.temp2,
            snapshot.flow1Lpm,
            snapshot.flow2Lpm,
            snapshot.faultActive,
            snapshot.timestampMs);
        vTaskDelay(delayTicks);
    }
}

static void WheelSpeedTask(void *parameter)
{
    (void)parameter;
    const TickType_t delayTicks = pdMS_TO_TICKS(20); // 50 Hz
    #if WATCHDOG_ENABLED
    esp_task_wdt_add(nullptr);
    #endif

    for (;;)
    {
        #if WATCHDOG_ENABLED
        esp_task_wdt_reset();
        #endif
        if (!SENSORS_ENABLED || !ENABLE_WHEEL_SPEED_SENSORS)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        SensorSnapshot snapshot = {};
        snapshot.timestampMs = millis();
        WheelSpeedReset();
        snapshot.speedFL = getWheelSpeedFL();
        snapshot.speedFR = getWheelSpeedFR();
        snapshot.speedRL = getWheelSpeedRL();
        snapshot.speedRR = getWheelSpeedRR();
        snapshot.speedKmh = getFinalWheelSpeed();
        if (CAN_ENABLED)
        {
            TelemetryService_SendWheelSpeed(snapshot.speedKmh);
        }

        SnapshotService_UpdateWheelSpeed(
            snapshot.speedFL,
            snapshot.speedFR,
            snapshot.speedRL,
            snapshot.speedRR,
            snapshot.speedKmh,
            snapshot.timestampMs);
        vTaskDelay(delayTicks);
    }
}

static void ChassisSensorsTask(void *parameter)
{
    (void)parameter;
    const TickType_t delayTicks = pdMS_TO_TICKS(50); // 20 Hz
    #if WATCHDOG_ENABLED
    esp_task_wdt_add(nullptr);
    #endif

    for (;;)
    {
        #if WATCHDOG_ENABLED
        esp_task_wdt_reset();
        #endif
        if (!SENSORS_ENABLED)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        SensorSnapshot snapshot = {};
        snapshot.timestampMs = millis();
        SensorService_ReadChassis(snapshot);
        SnapshotService_UpdateChassis(
            snapshot.susp1,
            snapshot.susp2,
            snapshot.susp3,
            snapshot.susp4,
            snapshot.steeringAngleDeg,
            snapshot.timestampMs);
        vTaskDelay(delayTicks);
    }
}

static void LoggerTask(void *parameter)
{
    (void)parameter;
    const TickType_t delayTicks = pdMS_TO_TICKS(20);
    #if WATCHDOG_ENABLED
    esp_task_wdt_add(nullptr);
    #endif

    for (;;)
    {
        #if WATCHDOG_ENABLED
        esp_task_wdt_reset();
        #endif
        static unsigned long lastSerialLogMs = 0;
        static unsigned long lastSdLogMs = 0;
        unsigned long now = millis();
        SensorSnapshot snapshot = SnapshotService_Read();
        if ((SYSTEM_MODE == MODE_FULL || SYSTEM_MODE == MODE_SENSORS_ONLY) &&
            now - lastSerialLogMs >= SERIAL_LOG_PERIOD_MS)
        {
            LoggingService_LogSnapshot(snapshot);
            lastSerialLogMs = now;
        }

        if (SYSTEM_MODE == MODE_FULL &&
            ENABLE_SD_LOGGING_OUTPUT &&
            now - lastSdLogMs >= SD_LOG_PERIOD_MS)
        {
            SDLoggingService_Append(snapshot);
            lastSdLogMs = now;
        }
        vTaskDelay(delayTicks);
    }
}

static void SimulatedSensorsTask(void *parameter)
{
    (void)parameter;
    const TickType_t delayTicks = pdMS_TO_TICKS(SENSOR_SIMULATION_PERIOD_MS);
#if WATCHDOG_ENABLED
    esp_task_wdt_add(nullptr);
#endif

    for (;;)
    {
#if WATCHDOG_ENABLED
        esp_task_wdt_reset();
#endif
        if (!SENSORS_ENABLED)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        SensorSnapshot snapshot = {};
        snapshot.timestampMs = millis();
        SimulatedSensors_FillSnapshot(snapshot, snapshot.timestampMs);
        snapshot.faultActive = FaultService_Update(snapshot.flow1Lpm, snapshot.flow2Lpm, snapshot.temp1, snapshot.temp2, CAN_ENABLED);

        SnapshotService_UpdateCritical(
            snapshot.temp1,
            snapshot.temp2,
            snapshot.flow1Lpm,
            snapshot.flow2Lpm,
            snapshot.faultActive,
            snapshot.timestampMs);
        SnapshotService_UpdateChassis(
            snapshot.susp1,
            snapshot.susp2,
            snapshot.susp3,
            snapshot.susp4,
            snapshot.steeringAngleDeg,
            snapshot.timestampMs);
        SnapshotService_UpdateWheelSpeed(
            snapshot.speedFL,
            snapshot.speedFR,
            snapshot.speedRL,
            snapshot.speedRR,
            snapshot.speedKmh,
            snapshot.timestampMs);

        if (CAN_ENABLED)
        {
            TelemetryService_SendWheelSpeed(snapshot.speedKmh);
        }

        vTaskDelay(delayTicks);
    }
}

static void TelemetryTask(void *parameter)
{
    (void)parameter;
    const TickType_t delayTicks = pdMS_TO_TICKS(TELEMETRY_PERIOD_MS);
#if WATCHDOG_ENABLED
    esp_task_wdt_add(nullptr);
#endif

    for (;;)
    {
#if WATCHDOG_ENABLED
        esp_task_wdt_reset();
#endif
        if (SYSTEM_MODE != MODE_FULL || !ENABLE_TELEMETRY_OUTPUT)
        {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        SensorSnapshot snapshot = SnapshotService_Read();
        TelemetryService_SendSnapshotCSV(snapshot);
        vTaskDelay(delayTicks);
    }
}

static void CanTestTask(void *parameter)
{
    (void)parameter;
    const TickType_t delayTicks = pdMS_TO_TICKS(10);
    #if WATCHDOG_ENABLED
    esp_task_wdt_add(nullptr);
    #endif

    for (;;)
    {
        #if WATCHDOG_ENABLED
        esp_task_wdt_reset();
        #endif
        if (SYSTEM_MODE != MODE_CAN_ONLY || !CAN_ENABLED)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        static unsigned long lastFaultSend = 0;
        TelemetryService_SendKnownWheelSpeedTestPattern();

        unsigned long now = millis();
        if (now - lastFaultSend >= 100) {
            CAN_SendUInt8(static_cast<uint16_t>(CANMessageId::CoolingFault), 1);
            lastFaultSend = now;
        }

        vTaskDelay(delayTicks);
    }
}

void Runtime_StartTasks()
{
    if (SYSTEM_MODE == MODE_CAN_ONLY)
    {
        xTaskCreatePinnedToCore(CanTestTask, "CanTest", 4096, nullptr, 4, nullptr, 0);
        xTaskCreatePinnedToCore(LoggerTask, "Logger", 4096, nullptr, 1, nullptr, 1);
        return;
    }

#if ENABLE_SENSOR_SIMULATION
    xTaskCreatePinnedToCore(SimulatedSensorsTask, "SimSensors", 4096, nullptr, 5, nullptr, 0);
    xTaskCreatePinnedToCore(LoggerTask, "Logger", 4096, nullptr, 1, nullptr, 1);
    #if ENABLE_TELEMETRY_OUTPUT
    xTaskCreatePinnedToCore(TelemetryTask, "Telemetry", 4096, nullptr, 1, nullptr, 1);
    #endif
    return;
#endif

    xTaskCreatePinnedToCore(CriticalSensorsTask, "CriticalSensors", 4096, nullptr, 5, nullptr, 0);
    xTaskCreatePinnedToCore(WheelSpeedTask, "WheelSpeed", 4096, nullptr, 4, nullptr, 0);
    xTaskCreatePinnedToCore(ChassisSensorsTask, "ChassisSensors", 4096, nullptr, 2, nullptr, 1);
    xTaskCreatePinnedToCore(LoggerTask, "Logger", 4096, nullptr, 1, nullptr, 1);
    #if ENABLE_TELEMETRY_OUTPUT
    xTaskCreatePinnedToCore(TelemetryTask, "Telemetry", 4096, nullptr, 1, nullptr, 1);
    #endif
}
