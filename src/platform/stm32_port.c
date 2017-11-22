//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "sys_timer.h"
#include "deca/deca_device_api.h"
#include "platform/stm32_port.h"
#include <string.h>
//------------------------------------------------------------------------------
// Macro's
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Function prototypes
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Global variables
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Public functions
//------------------------------------------------------------------------------

static SPI_HandleTypeDef *spiHandle;

void port_init(SPI_HandleTypeDef *handle)
{
    spiHandle = handle;
}

int writetospi(uint16 headerLength, const uint8 *headerBuffer, uint32 bodylength,
        const uint8 *bodyBuffer)
{
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
    HAL_SPI_Transmit(spiHandle, (uint8_t *) headerBuffer, headerLength, 1000);
    HAL_StatusTypeDef ret = HAL_SPI_Transmit(spiHandle, (uint8_t *) bodyBuffer, bodylength, 1000);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
    return ret;
}

int readfromspi(uint16 headerLength, const uint8 *headerBuffer, uint32 readlength,
        uint8 *readBuffer)
{
    for(int i = 0; i < readlength; ++i) {
        readBuffer[i] = 0;
    }
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_RESET);
    HAL_SPI_Transmit(spiHandle, (uint8_t *) headerBuffer, headerLength, 1000);
    HAL_StatusTypeDef ret = HAL_SPI_Receive(spiHandle, readBuffer, readlength, 1000);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_14, GPIO_PIN_SET);
    return ret; 
}

// FIXME: Disabling all interrupts might lead to problems
decaIrqStatus_t decamutexon(void)
{
    __disable_irq();
    return 1;
}

void decamutexoff(decaIrqStatus_t s)
{
    if(s) {
        __enable_irq();
    }
}

void deca_sleep(unsigned int time_ms)
{
    sys_timer_delay_ms(time_ms);
}