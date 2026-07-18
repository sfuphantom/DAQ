#include "snapshotService.h"

static SensorSnapshot gSnapshot = {};
static portMUX_TYPE gSnapshotMux = portMUX_INITIALIZER_UNLOCKED;

void snapshotServiceUpdateCritical(float temp1, float temp2, float flow1Lpm, float flow2Lpm, bool faultActive, uint32_t timestampMs) {
    portENTER_CRITICAL(&gSnapshotMux);
    gSnapshot.temp1 = temp1;
    gSnapshot.temp2 = temp2;
    gSnapshot.flow1Lpm = flow1Lpm;
    gSnapshot.flow2Lpm = flow2Lpm;
    gSnapshot.faultActive = faultActive;
    gSnapshot.criticalTimestampMs = timestampMs;
    portEXIT_CRITICAL(&gSnapshotMux);
}

void snapshotServiceUpdateWheelSpeed(float speedFL, float speedFR, float speedRL, float speedRR, float speedKmh, uint32_t timestampMs) {
    portENTER_CRITICAL(&gSnapshotMux);
    gSnapshot.speedFL = speedFL;
    gSnapshot.speedFR = speedFR;
    gSnapshot.speedRL = speedRL;
    gSnapshot.speedRR = speedRR;
    gSnapshot.speedKmh = speedKmh;
    gSnapshot.wheelSpeedTimestampMs = timestampMs;
    portEXIT_CRITICAL(&gSnapshotMux);
}

void snapshotServiceUpdateChassis(float susp1, float susp2, float susp3, float susp4, float steeringAngleDeg, uint32_t timestampMs) {
    portENTER_CRITICAL(&gSnapshotMux);
    gSnapshot.susp1 = susp1;
    gSnapshot.susp2 = susp2;
    gSnapshot.susp3 = susp3;
    gSnapshot.susp4 = susp4;
    gSnapshot.steeringAngleDeg = steeringAngleDeg;
    gSnapshot.chassisTimestampMs = timestampMs;
    portEXIT_CRITICAL(&gSnapshotMux);
}

SensorSnapshot snapshotServiceRead() {
    SensorSnapshot snapshot;
    portENTER_CRITICAL(&gSnapshotMux);
    snapshot = gSnapshot;
    portEXIT_CRITICAL(&gSnapshotMux);
    return snapshot;
}
