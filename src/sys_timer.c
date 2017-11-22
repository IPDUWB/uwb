//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <stdbool.h>
#include "sys_timer.h"
#include "system_stm32f4xx.h"
#include "stm32f4xx.h"
#include "core_cm4.h"

//------------------------------------------------------------------------------
// Macro's
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Global variables
//------------------------------------------------------------------------------

static volatile uint32_t time = 0;
static volatile uint32_t delayMs = 0;

//------------------------------------------------------------------------------
// Private functions
//------------------------------------------------------------------------------

void SysTick_Handler(void)
{
    time++;
    if((delayMs != 0) && (time == delayMs))
    {
        asm volatile("sev");
    }
}

//------------------------------------------------------------------------------
// Public functions
//------------------------------------------------------------------------------

void sys_timer_init(void)
{
    SysTick_Config(SystemCoreClock / 1000);
}

uint32_t sys_timer_get_ms(void)
{
    return time;
}

void sys_timer_delay_ms(uint32_t delay)
{
    delayMs = time + delay;
    while(time < delayMs)
    {
        asm volatile("wfe");
    }
    delayMs = 0;
}

uint32_t sys_timer_get_elapsed_ms(uint32_t start)
{
    uint32_t now = time;
    uint32_t max = UINT32_MAX;
    return (now >= start) ? (now - start) :
           (max - start + 1 + now);
}

void sys_timer_destroy(void)
{
}

uint32_t HAL_GetTick(void)
{
    return time;
}

void HAL_Delay(__IO uint32_t Delay)
{
    sys_timer_delay_ms(Delay);
}

