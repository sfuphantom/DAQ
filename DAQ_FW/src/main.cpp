#include "logger.h"
#include "faultService.h"
#include "rtcService.h"
#include "sdLoggingService.h"
#include "sensorService.h"
#include "taskScheduler.h"
#include "wheelSpeed.h"
#include "can.h"
#include "systemConfig.h"
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>

void setup() {
    Serial.begin(BAUD_RATE);
    delay(1000);
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    Logger::start();
    #if WATCHDOG_ENABLED
    esp_task_wdt_init(WATCHDOG_TIMEOUT_S, true);
    #endif
    rtcServiceInit();

    bool sdOk = false;
    #if ENABLE_SD_LOGGING_OUTPUT
    SPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
    sdOk = SD.begin(SD_CS_PIN, SPI);
    if (!sdOk) {
        Logger::error("SD init failed");
    } else {
#if ENABLE_STATUS_LOGS
        Logger::notice("SD init OK");
#endif
    }
    #endif
    char runTimestamp[16] = "";
    rtcServiceGetBootTimestamp(runTimestamp, sizeof(runTimestamp));
    sdLoggingServiceInit(sdOk, runTimestamp);

    #if ENABLE_TELEMETRY_OUTPUT
    TELEMETRY_UART.begin(TELEMETRY_BAUD, SERIAL_8N1, TELEMETRY_RX_PIN, TELEMETRY_TX_PIN);
#if ENABLE_STATUS_LOGS
    Logger::notice("Telemetry UART init OK");
#endif
    #endif
    if (CAN_ENABLED) {
        bool canOk = canInit();
        Serial.println();
        if (canOk) {
#if ENABLE_STATUS_LOGS
            Logger::notice("CAN initialized");
#endif
        } else {
            Logger::error("CAN unavailable");
        }
    }

    #if ENABLE_WHEEL_SPEED_SENSORS && !ENABLE_SENSOR_SIMULATION
    if (SENSORS_ENABLED) {
        wheelSpeedSetup();
    }
    #endif

    #if ENABLE_SENSOR_SIMULATION
    if (SENSORS_ENABLED) {
#if ENABLE_STATUS_LOGS
        Logger::notice("Sensor simulation enabled");
#endif
    }
    #endif

    if (SENSORS_ENABLED && !ENABLE_SENSOR_SIMULATION) {
        sensorServiceInit();
    }

    // Send initialization message
    #if CAN_ENABLED
        canSendUInt8(static_cast<uint16_t>(CANMessageId::CoolingFault), 0);
    #endif
#if ENABLE_STATUS_LOGS
    Logger::notice("Setup complete");
    Logger::notice("Starting main loop...");
#endif
    Serial.println();

    faultServiceInit(millis());
    runtimeStartTasks();
}


void loop() {
    // idle due to task driven design, tasks are scheduled in runtimeStartTasks()
    vTaskDelay(pdMS_TO_TICKS(1000));
}
