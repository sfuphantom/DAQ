#ifndef CAN_LIB
#define CAN_LIB
#include "driver/gpio.h"
#include "driver/can.h"
#include "systemConfig.h"

namespace CanDriver {
    void canInit();

    void sendCanData(const char *canData, const uint32_t canLen, const uint16_t canID, const int canDATAint, bool isStringmsg);

    void readCanData();
}

#endif
