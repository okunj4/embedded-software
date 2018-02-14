#include <stdint.h>
#include <stdbool.h>

#include "system.h"
#include "macros.h"
#include <sysclk.h>

/* Initializes a timer peripheral with a period
of 1ms. For Cortex-M cores we can use the built-in
SysTick interrupt
*/
void hal_Timing_Init(void){
    SysTick_Config(FCPU/1000);
}


void SysTick_Handler (void){
    TimingISR();
}

#define US_PER_TICK 1000
#define TimingUsNow() (time_ms*US_PER_TICK)

