#ifndef CAN_H
#define CAN_H

#include <driver/twai.h>
#include <stdint.h>

void CAN_Init();
void CAN_SendInt16(uint16_t id, int16_t value);

#endif
