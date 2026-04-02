#include "dwt_delay.h"
#include "main.h"

void DWT_Init() {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

void DWT_Delay(uint32_t sys_clock, uint32_t us) {
    uint32_t startTick = DWT->CYCCNT;
    uint32_t delayTicks = us * (sys_clock / 1000000);

    while (DWT->CYCCNT - startTick < delayTicks);
}

uint32_t DWT_GetTick() {
    return DWT->CYCCNT;
}

