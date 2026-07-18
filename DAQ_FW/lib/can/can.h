#ifndef CAN_H
#define CAN_H

#include <driver/twai.h>
#include <stdint.h>

bool canInit();
void canSendUInt8(uint16_t id, uint8_t value);
void canSendInt16(uint16_t id, int16_t value);

#endif
