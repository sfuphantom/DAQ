#include "taskScheduler.h"
#include "faultService.h"
#include "loggingService.h"
#include "sdLoggingService.h"
#include "sensorService.h"
#include "snapshotService.h"
#include "simulatedSensors.h"
#include "telemetryService.h"
#include "can.h"
#include "logger.h"
#include "systemConfig.h"
#include "wheelSpeed.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <math.h>
#include <esp_task_wdt.h>

/*
 This file defines and starts the system's periodic FreeRTOS runtime tasks.
*/

static void criticalSensorsTask(void *parameter) {
    (void)parameter;
    const TickType_t delayTicks = pdMS_TO_TICKS(20); // 50 Hz
    #if WATCHDOG_ENABLED
    esp_task_wdt_add(nullptr);
    #endif

    for (;;) {
        #if WATCHDOG_ENABLED
        esp_task_wdt_reset();
        #endif
        if (!SENSORS_ENABLED) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        SensorSnapshot snapshot = {};
        uint32_t timestampMs = millis();
        sensorServiceReadCritical(snapshot);
        snapshot.faultActive = faultServiceUpdate(snapshot.flow1Lpm, snapshot.flow2Lpm, snapshot.temp1, snapshot.temp2, CAN_ENABLED);

        snapshotServiceUpdateCritical(
            snapshot.temp1,
            snapshot.temp2,
            snapshot.flow1Lpm,
            snapshot.flow2Lpm,
            snapshot.faultActive,
            timestampMs);
        vTaskDelay(delayTicks);
    }
}

static void wheelSpeedTask(void *parameter) {
    (void)parameter;
    const TickType_t delayTicks = pdMS_TO_TICKS(20); // 50 Hz
    #if WATCHDOG_ENABLED
    esp_task_wdt_add(nullptr);
    #endif

    for (;;) {
        #if WATCHDOG_ENABLED
        esp_task_wdt_reset();
        #endif
        if (!SENSORS_ENABLED || !ENABLE_WHEEL_SPEED_SENSORS) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        SensorSnapshot snapshot = {};
        uint32_t timestampMs = millis();
        wheelSpeedReset();
        snapshot.speedFL = getWheelSpeedFl();
        snapshot.speedFR = getWheelSpeedFr();
        snapshot.speedRL = getWheelSpeedRl();
        snapshot.speedRR = getWheelSpeedRr();
        snapshot.speedKmh = getFinalWheelSpeed();
        if (CAN_ENABLED) {
            telemetryServiceSendWheelSpeed(snapshot.speedKmh);
        }

        snapshotServiceUpdateWheelSpeed(
            snapshot.speedFL,
            snapshot.speedFR,
            snapshot.speedRL,
            snapshot.speedRR,
            snapshot.speedKmh,
            timestampMs);
        vTaskDelay(delayTicks);
    }
}

static void chassisSensorsTask(void *parameter) {
    (void)parameter;
    const TickType_t delayTicks = pdMS_TO_TICKS(10); // 100 Hz
    #if WATCHDOG_ENABLED
    esp_task_wdt_add(nullptr);
    #endif

    for (;;) {
        #if WATCHDOG_ENABLED
        esp_task_wdt_reset();
        #endif
        if (!SENSORS_ENABLED) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        SensorSnapshot snapshot = {};
        uint32_t timestampMs = millis();
        sensorServiceReadChassis(snapshot);
        snapshotServiceUpdateChassis(
            snapshot.susp1,
            snapshot.susp2,
            snapshot.susp3,
            snapshot.susp4,
            snapshot.steeringAngleDeg,
            timestampMs);
        vTaskDelay(delayTicks);
    }
}

static void loggerTask(void *parameter) {
    (void)parameter;
    const TickType_t delayTicks = pdMS_TO_TICKS(20);
    #if WATCHDOG_ENABLED
    esp_task_wdt_add(nullptr);
    #endif

    for (;;) {
        #if WATCHDOG_ENABLED
        esp_task_wdt_reset();
        #endif
        static unsigned long lastSerialLogMs = 0;
        static unsigned long lastSdLogMs = 0;
        unsigned long now = millis();
        SensorSnapshot snapshot = snapshotServiceRead();
        if ((SYSTEM_MODE == MODE_FULL || SYSTEM_MODE == MODE_SENSORS_ONLY) &&
            now - lastSerialLogMs >= SERIAL_LOG_PERIOD_MS) {
            loggingServiceLogSnapshot(snapshot);
            lastSerialLogMs = now;
        }

        if (SYSTEM_MODE == MODE_FULL &&
            ENABLE_SD_LOGGING_OUTPUT &&
            now - lastSdLogMs >= SD_LOG_PERIOD_MS) {
            sdLoggingServiceAppend(snapshot);
            lastSdLogMs = now;
        }
        vTaskDelay(delayTicks);
    }
}

static void simulatedSensorsTask(void *parameter) {
    (void)parameter;
    const TickType_t delayTicks = pdMS_TO_TICKS(SENSOR_SIMULATION_PERIOD_MS);
#if WATCHDOG_ENABLED
    esp_task_wdt_add(nullptr);
#endif

    for (;;) {
#if WATCHDOG_ENABLED
        esp_task_wdt_reset();
#endif
        if (!SENSORS_ENABLED) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        SensorSnapshot snapshot = {};
        uint32_t timestampMs = millis();
        simulatedSensorsFillSnapshot(snapshot, timestampMs);
        snapshot.faultActive = faultServiceUpdate(snapshot.flow1Lpm, snapshot.flow2Lpm, snapshot.temp1, snapshot.temp2, CAN_ENABLED);

        snapshotServiceUpdateCritical(
            snapshot.temp1,
            snapshot.temp2,
            snapshot.flow1Lpm,
            snapshot.flow2Lpm,
            snapshot.faultActive,
            timestampMs);
        snapshotServiceUpdateChassis(
            snapshot.susp1,
            snapshot.susp2,
            snapshot.susp3,
            snapshot.susp4,
            snapshot.steeringAngleDeg,
            timestampMs);
        snapshotServiceUpdateWheelSpeed(
            snapshot.speedFL,
            snapshot.speedFR,
            snapshot.speedRL,
            snapshot.speedRR,
            snapshot.speedKmh,
            timestampMs);

        if (CAN_ENABLED) {
            telemetryServiceSendWheelSpeed(snapshot.speedKmh);
        }

        vTaskDelay(delayTicks);
    }
}

static void telemetryTask(void *parameter) {
    (void)parameter;
    const TickType_t delayTicks = pdMS_TO_TICKS(TELEMETRY_PERIOD_MS);
#if WATCHDOG_ENABLED
    esp_task_wdt_add(nullptr);
#endif

    for (;;) {
#if WATCHDOG_ENABLED
        esp_task_wdt_reset();
#endif
        if (SYSTEM_MODE != MODE_FULL || !ENABLE_TELEMETRY_OUTPUT) {
            vTaskDelay(pdMS_TO_TICKS(100));
            continue;
        }

        SensorSnapshot snapshot = snapshotServiceRead();
        telemetryServiceSendSnapshotCsv(snapshot);
        vTaskDelay(delayTicks);
    }
}

static void canTestTask(void *parameter) {
    (void)parameter;
    const TickType_t delayTicks = pdMS_TO_TICKS(10);
    #if WATCHDOG_ENABLED
    esp_task_wdt_add(nullptr);
    #endif

    for (;;) {
        #if WATCHDOG_ENABLED
        esp_task_wdt_reset();
        #endif
        if (SYSTEM_MODE != MODE_CAN_ONLY || !CAN_ENABLED) {
            vTaskDelay(pdMS_TO_TICKS(1000));
            continue;
        }

        static unsigned long lastFaultSend = 0;
        telemetryServiceSendKnownWheelSpeedTestPattern();

        unsigned long now = millis();
        if (now - lastFaultSend >= 100) {
            canSendUInt8(static_cast<uint16_t>(CANMessageId::CoolingFault), 1);
            lastFaultSend = now;
        }

        vTaskDelay(delayTicks);
    }
}

static bool startTask(TaskFunction_t taskFunction, const char *taskName, uint32_t stackDepth, UBaseType_t priority, BaseType_t coreId) {
    BaseType_t result = xTaskCreatePinnedToCore(
        taskFunction,
        taskName,
        stackDepth,
        nullptr,
        priority,
        nullptr,
        coreId);

    if (result != pdPASS) {
        Logger::fatal(
            "Failed to start task %s (stack=%lu priority=%u core=%d result=%ld)",
            taskName,
            static_cast<unsigned long>(stackDepth),
            static_cast<unsigned>(priority),
            static_cast<int>(coreId),
            static_cast<long>(result));
        return false;
    }

#if ENABLE_STATUS_LOGS
    Logger::notice(
        "Started task %s (stack=%lu priority=%u core=%d)",
        taskName,
        static_cast<unsigned long>(stackDepth),
        static_cast<unsigned>(priority),
        static_cast<int>(coreId));
#endif
    return true;
}

void runtimeStartTasks() {
    if (SYSTEM_MODE == MODE_CAN_ONLY) {
        startTask(canTestTask, "CanTest", 4096, 4, 0);
        startTask(loggerTask, "Logger", 4096, 1, 1);
        return;
    }

#if ENABLE_SENSOR_SIMULATION
    startTask(simulatedSensorsTask, "SimSensors", 4096, 5, 0);
    startTask(loggerTask, "Logger", 4096, 1, 1);
    #if ENABLE_TELEMETRY_OUTPUT
    startTask(telemetryTask, "Telemetry", 4096, 1, 1);
    #endif
    return;
#endif

    startTask(criticalSensorsTask, "CriticalSensors", 4096, 5, 0);
    startTask(wheelSpeedTask, "WheelSpeed", 4096, 4, 0);
    startTask(chassisSensorsTask, "ChassisSensors", 4096, 2, 1);
    startTask(loggerTask, "Logger", 4096, 1, 1);
    #if ENABLE_TELEMETRY_OUTPUT
    startTask(telemetryTask, "Telemetry", 4096, 1, 1);
    #endif
}
