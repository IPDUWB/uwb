//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
//  stdlib
#include <stdio.h>
#include <stdbool.h>
// HAL
#include "stm32f4xx_hal.h"
// Decawave
#include "deca/deca_device_api.h"
#include "deca/deca_regs.h"
// Project
#include "resources.h"
#include "platform/stm32_port.h"
#include "platform/stm32_init.h"

//------------------------------------------------------------------------------
// Macro's
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Global variables
//------------------------------------------------------------------------------

static dwt_config_t config = {
        .chan = 7,                      // Channel number.
        .prf = DWT_PRF_64M,             // Pulse repetition frequency.
        .txPreambLength = DWT_PLEN_64,  // Preamble length. Used in TX only.
        .rxPAC = DWT_PAC8,             // Preamble acquisition chunk size. Used in RX only.
        .txCode = 17,                   // TX preamble code. Used in TX only.
        .rxCode = 17,                   // RX preamble code. Used in RX only.
        .nsSFD = true,                 // 0 to use standard SFD, 1 to use non-standard SFD.
        .dataRate = DWT_BR_6M8,         // Data rate.
        .phrMode = DWT_PHRMODE_EXT,     // PHY header mode.
        .sfdTO = (65 + 16 - 8)           // SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only.
};

//------------------------------------------------------------------------------
// Functions
//------------------------------------------------------------------------------

int main(void)
{
    HAL_Init();
    system_clock_config();

    __HAL_RCC_GPIOB_CLK_ENABLE();

    // Enable GPIO used for DW1000 reset
    GPIO_InitTypeDef gpioBlinkConfig = {
            .Pin = GPIO_PIN_7,
            .Mode = GPIO_MODE_OUTPUT_PP,
            .Pull = GPIO_PULLDOWN,
            .Speed = GPIO_SPEED_MEDIUM
    };
    HAL_GPIO_Init(GPIOB, &gpioBlinkConfig);;

    SPI_HandleTypeDef spiHandle = spi_init();
    port_init(&spiHandle);
    dw_reset();

    if(dwt_initialise(DWT_LOADNONE) == DWT_ERROR) {
        while(1);
    }

    dwt_loadopsettabfromotp(DWT_OPSET_64LEN);

    // Set SPI frequency to 20Mhz
    SPI1->CR1 |= SPI_BAUDRATEPRESCALER_4;
    dwt_configure(&config);

    while(true) {
        // Write frame data to DW1000 and prepare transmission. See NOTE 4 below.
        dwt_writetxdata(sizeof(tx_msg), tx_msg, 0);
        // Zero offset in TX buffer, no ranging.
        dwt_writetxfctrl(sizeof(tx_msg), 0, 0);

        // Start transmission.
        dwt_starttx(DWT_START_TX_IMMEDIATE);

        // Poll DW1000 until TX frame sent event set.
        while (!(dwt_read32bitreg(SYS_STATUS_ID) & SYS_STATUS_TXFRS));

        // Clear TX frame sent event.
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_TXFRS);
    }
    return 0;
}
