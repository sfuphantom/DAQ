#ifndef IMUSENSOR_LIB
#define IMUSENSOR_LIB

#include "Adafruit_LSM6DS3TRC.h"

class IMU_Sensor : public Adafruit_LSM6DS3TRC {
public:
    IMU_Sensor();

    void initialize();
    void getTempAccelGyroData(Adafruit_LSM6DS3TRC lsm6ds3trc);

private: 
    Adafruit_LSM6DS3TRC lsm6ds3trc;
};

#endif
