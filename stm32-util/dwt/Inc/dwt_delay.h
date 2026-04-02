#ifndef DWT_DELAY_H_
#define DWT_DELAY_H_

#include <stdint.h>

void DWT_Init(void);
void DWT_Delay(uint32_t sys_clock, uint32_t us);
uint32_t DWT_GetTick();

#endif
