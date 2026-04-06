#include "Logger.h"
#include "FaultService.h"
#include "SDLoggingService.h"
#include "SensorService.h"
#include "TaskScheduler.h"
#include "wheelSpeed.h"
#include "can.h"
#include "system_config.h"
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>

void setup()
{
    Serial.begin(BAUD_RATE);
    delay(1000);
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN); 

    Logger::Start();
    #if WATCHDOG_ENABLED
    esp_task_wdt_init(WATCHDOG_TIMEOUT_S, true);
    #endif
    bool sdOk = false;
    #if ENABLE_SD_LOGGING_OUTPUT
    SPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
    sdOk = SD.begin(SD_CS_PIN, SPI);
    if (!sdOk) {
        Logger::Error("SD init failed");
    } else {
        Logger::Notice("SD init OK");
    }
    #endif
    SDLoggingService_Init(sdOk);

    #if ENABLE_TELEMETRY_OUTPUT
    TELEMETRY_UART.begin(TELEMETRY_BAUD, SERIAL_8N1, TELEMETRY_RX_PIN, TELEMETRY_TX_PIN);
    #endif
    if (CAN_ENABLED)
    {
        CAN_Init();
        Serial.println();
        Logger::Notice("CAN initialized");
    }

    #if ENABLE_WHEEL_SPEED_SENSORS && !ENABLE_SENSOR_SIMULATION
    if (SENSORS_ENABLED)
    {
        WheelSpeedSetup();
    }
    #endif

    #if ENABLE_SENSOR_SIMULATION
    if (SENSORS_ENABLED)
    {
        Logger::Notice("Sensor simulation enabled");
    }
    #endif

    if (SENSORS_ENABLED && !ENABLE_SENSOR_SIMULATION)
    {
        SensorService_Init();
    }

    // Send initialization message
    #if CAN_ENABLED
        CAN_SendUInt8(static_cast<uint16_t>(CANMessageId::CoolingFault), 0);
    #endif
    Logger::Notice("Setup complete");
    Logger::Notice("Starting main loop...");
    Serial.println();

    FaultService_Init(millis());
    Runtime_StartTasks();
}


void loop()
{
    // idle due to task driven design, tasks are scheduled in Runtime_StartTasks()
    vTaskDelay(pdMS_TO_TICKS(1000));
}
