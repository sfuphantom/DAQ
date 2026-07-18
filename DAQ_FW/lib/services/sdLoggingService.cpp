#include "sdLoggingService.h"
#include "logger.h"
#include "rtcService.h"
#include "systemConfig.h"
#include <Arduino.h>
#include <SD.h>
#include <math.h>
#include <stdio.h>

namespace {
    bool sdReady = false;
    bool headerWritten = false;
    uint32_t rowsWritten = 0;
    const char *kLogDir = "/unprocessed";
    char logPath[64] = "/unprocessed/log_0001.csv";
}

static const char *formatValue(float value, char *buffer, size_t length, uint8_t precision) {
    if (isnan(value)) {
        snprintf(buffer, length, "%s", SENSOR_NULL_TEXT);
    }
    else {
        snprintf(buffer, length, "%.*f", precision, value);
    }
    return buffer;
}

void sdLoggingServiceInit(bool sdAvailable, const char *runTimestamp) {
    sdReady = sdAvailable;
    headerWritten = false;
    rowsWritten = 0;
    if (!sdReady) {
#if ENABLE_STATUS_LOGS
        Logger::notice("SD logging disabled");
#endif
        return;
    }

    if (!SD.exists(kLogDir)) {
        if (!SD.mkdir(kLogDir)) {
            Logger::error("SD log directory create failed: %s", kLogDir);
        }
    }

    if (runTimestamp != nullptr && runTimestamp[0] != '\0') {
        snprintf(logPath, sizeof(logPath), "%s/run_%s.csv", kLogDir, runTimestamp);
        if (!SD.exists(logPath)) {
#if ENABLE_STATUS_LOGS
            Logger::notice("SD log path: %s", logPath);
#endif
            return;
        }

        for (uint8_t i = 1; i <= 99; ++i) {
            snprintf(logPath, sizeof(logPath), "%s/run_%s_%02u.csv", kLogDir, runTimestamp, static_cast<unsigned>(i));
            if (!SD.exists(logPath)) {
#if ENABLE_STATUS_LOGS
                Logger::notice("SD log path: %s", logPath);
#endif
                return;
            }
        }
    }

    for (uint16_t i = 1; i <= 9999; ++i) {
        snprintf(logPath, sizeof(logPath), "%s/log_%04u.csv", kLogDir, static_cast<unsigned>(i));
        if (!SD.exists(logPath)) {
#if ENABLE_STATUS_LOGS
            Logger::notice("SD log path: %s", logPath);
#endif
            break;
        }
    }
}

void sdLoggingServiceAppend(const SensorSnapshot &snapshot) {
    if (!sdReady) {
        return;
    }

    File file = SD.open(logPath, FILE_APPEND);
    if (!file) {
        Logger::error("SD log open failed: %s", logPath);
        return;
    }

    if (!headerWritten) {
        file.println("critical_timestamp_ms,chassis_timestamp_ms,wheel_speed_timestamp_ms,rtc_time,temp1_c,temp2_c,flow1_lpm,flow2_lpm,susp1_v,susp2_v,susp3_v,susp4_v,steering_angle_deg,speed_fl_kmh,speed_fr_kmh,speed_rl_kmh,speed_rr_kmh,speed_kmh,fault_active");
        headerWritten = true;
#if ENABLE_STATUS_LOGS
        Logger::notice("SD log header written: %s", logPath);
#endif
    }

    char rtcTimeBuffer[24];
    char temp1Buffer[16];
    char temp2Buffer[16];
    char flow1Buffer[16];
    char flow2Buffer[16];
    char susp1Buffer[16];
    char susp2Buffer[16];
    char susp3Buffer[16];
    char susp4Buffer[16];
    char steeringAngleBuffer[16];
    char speedFLBuffer[16];
    char speedFRBuffer[16];
    char speedRLBuffer[16];
    char speedRRBuffer[16];
    char speedBuffer[16];

    file.print(snapshot.criticalTimestampMs);
    file.print(",");
    file.print(snapshot.chassisTimestampMs);
    file.print(",");
    file.print(snapshot.wheelSpeedTimestampMs);
    file.print(",");
    if (rtcServiceGetCurrentTimestamp(rtcTimeBuffer, sizeof(rtcTimeBuffer))) {
        file.print(rtcTimeBuffer);
    }
    else {
        file.print(SENSOR_NULL_TEXT);
    }
    file.print(",");
    file.print(formatValue(snapshot.temp1, temp1Buffer, sizeof(temp1Buffer), 1));
    file.print(",");
    file.print(formatValue(snapshot.temp2, temp2Buffer, sizeof(temp2Buffer), 1));
    file.print(",");
    file.print(formatValue(snapshot.flow1Lpm, flow1Buffer, sizeof(flow1Buffer), 2));
    file.print(",");
    file.print(formatValue(snapshot.flow2Lpm, flow2Buffer, sizeof(flow2Buffer), 2));
    file.print(",");
    file.print(formatValue(snapshot.susp1, susp1Buffer, sizeof(susp1Buffer), 3));
    file.print(",");
    file.print(formatValue(snapshot.susp2, susp2Buffer, sizeof(susp2Buffer), 3));
    file.print(",");
    file.print(formatValue(snapshot.susp3, susp3Buffer, sizeof(susp3Buffer), 3));
    file.print(",");
    file.print(formatValue(snapshot.susp4, susp4Buffer, sizeof(susp4Buffer), 3));
    file.print(",");
    file.print(formatValue(snapshot.steeringAngleDeg, steeringAngleBuffer, sizeof(steeringAngleBuffer), 1));
    file.print(",");
    file.print(formatValue(snapshot.speedFL, speedFLBuffer, sizeof(speedFLBuffer), 2));
    file.print(",");
    file.print(formatValue(snapshot.speedFR, speedFRBuffer, sizeof(speedFRBuffer), 2));
    file.print(",");
    file.print(formatValue(snapshot.speedRL, speedRLBuffer, sizeof(speedRLBuffer), 2));
    file.print(",");
    file.print(formatValue(snapshot.speedRR, speedRRBuffer, sizeof(speedRRBuffer), 2));
    file.print(",");
    file.print(formatValue(snapshot.speedKmh, speedBuffer, sizeof(speedBuffer), 2));
    file.print(",");
    file.println(snapshot.faultActive ? 1 : 0);

    file.close();
    ++rowsWritten;

    if (rowsWritten == 1 || rowsWritten % 100 == 0) {
#if ENABLE_STATUS_LOGS
        Logger::notice("SD log rows written: %lu (%s)", static_cast<unsigned long>(rowsWritten), logPath);
#endif
    }
}

const char *sdLoggingServiceLogPath() {
    return logPath;
}
