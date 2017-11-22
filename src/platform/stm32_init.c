/*
 * Created by bart452 on 22/11/17.
 */


//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
#include "platform/stm32_init.h"
// Project
#include "sys_timer.h"

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
void dw_reset(void)
{
    __HAL_RCC_GPIOE_CLK_ENABLE();
    // Enable GPIO used for DW1000 reset
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.Pin = GPIO_PIN_13;
    GPIO_InitStructure.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStructure.Speed = GPIO_SPEED_MEDIUM;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);

    //drive the RSTn pin low
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_RESET);

    sys_timer_delay_ms(2);

    //put the pin back to tri-state ... as input
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_13);
    GPIO_InitStructure.Mode = GPIO_MODE_ANALOG;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStructure);
    sys_timer_delay_ms(5);

}

void system_clock_config(void)
{

    SystemInit();
    /* Enable Power Control clock */
    __HAL_RCC_PWR_CLK_ENABLE();

    /* The voltage scaling allows optimizing the power consumption when the device is
       clocked below the maximum system frequency, to update the voltage scaling value
       regarding system frequency refer to product datasheet.  */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /* Enable HSE Oscillator and activate PLL with HSE as source */
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 160;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    RCC_OscInitStruct.PLL.PLLR = 2;
    HAL_StatusTypeDef ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);

    if(ret != HAL_OK)
    {
        while(1) { ; }
    }

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
       clocks dividers */
    RCC_ClkInitTypeDef rccClockConfig;
    rccClockConfig.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    rccClockConfig.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    rccClockConfig.AHBCLKDivider = RCC_SYSCLK_DIV1;
    rccClockConfig.APB1CLKDivider = RCC_HCLK_DIV2;
    rccClockConfig.APB2CLKDivider = RCC_HCLK_DIV1;
    ret = HAL_RCC_ClockConfig(&rccClockConfig, FLASH_LATENCY_3);
    if(ret != HAL_OK)
    {
        while(1) { ; }
    }
}

SPI_HandleTypeDef spi_init(void)
{

    /* Peripheral Clock Enable -------------------------------------------------*/
    // Enable the SPI clock en GPIO clocks
    __HAL_RCC_SPI1_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    SPI_HandleTypeDef spiHandle = {
            .Instance               = SPI1,
            .Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256,
            .Init.Direction         = SPI_DIRECTION_2LINES,
            .Init.CLKPhase          = SPI_PHASE_1EDGE,
            .Init.CLKPolarity       = SPI_POLARITY_LOW,
            .Init.DataSize          = SPI_DATASIZE_8BIT,
            .Init.FirstBit          = SPI_FIRSTBIT_MSB,
            .Init.TIMode            = SPI_TIMODE_DISABLE,
            .Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE,
            .Init.CRCPolynomial     = 7,
            .Init.NSS               = SPI_NSS_SOFT,
            .Init.Mode = SPI_MODE_MASTER,
    };

    if(HAL_SPI_Init(&spiHandle) != HAL_OK)
    {
        /* Initialization Error */
        while(1);
    }

    GPIO_InitTypeDef gpioConfig = {
            .Pin       = GPIO_PIN_5,
            .Mode      = GPIO_MODE_AF_PP,
            .Pull      = GPIO_PULLDOWN,
            .Speed     = GPIO_SPEED_HIGH,
            .Alternate = GPIO_AF5_SPI1,
    };
    HAL_GPIO_Init(GPIOA, &gpioConfig);

    /* SPI MISO GPIO pin configuration  */
    gpioConfig.Pin = GPIO_PIN_6;
    gpioConfig.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &gpioConfig);

    /* SPI MOSI GPIO pin configuration  */
    gpioConfig.Pin = GPIO_PIN_7;
    gpioConfig.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &gpioConfig);

    gpioConfig.Pin = GPIO_PIN_14;
    gpioConfig.Mode = GPIO_MODE_OUTPUT_PP;
    gpioConfig.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOD, &gpioConfig);

    /*##-3- Configure the NVIC for SPI #########################################*/
    /* NVIC for SPI */
    HAL_NVIC_SetPriority(SPI1_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(SPI1_IRQn);

    return spiHandle;
}

//------------------------------------------------------------------------------
// Private functions
//------------------------------------------------------------------------------
