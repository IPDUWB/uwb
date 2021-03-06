//------------------------------------------------------------------------------
// Includes
//------------------------------------------------------------------------------
//  stdlib
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
// HAL
#include "stm32f4xx_hal.h"
// Decawave
#include "deca/deca_device_api.h"
#include "deca/deca_regs.h"
// Project
#include "platform/stm32_port.h"
#include "platform/stm32_init.h"
#include "sys_timer.h"

//------------------------------------------------------------------------------
// Macro's
//------------------------------------------------------------------------------

#define FRAME_RECEIVE_LEN_MAX    1023

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
        .sfdTO = (65 + 8 - 8)           // SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only.
};

static uint8_t rx_buffer[FRAME_RECEIVE_LEN_MAX] = {0};

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

    uint32_t time = 0;
    uint32_t totalFrame = 0;
    while (true) {

        // Clear buffer
        memset(rx_buffer, 0, sizeof(rx_buffer));

        // Activate reception immediately. See NOTE 4 below.
        dwt_rxenable(DWT_START_RX_IMMEDIATE);

        // Poll until a frame is properly received or an error/timeout occurs
        uint32_t status_reg = 0;
        uint32_t frame_len = 0;

        while(!((status_reg = dwt_read32bitreg(SYS_STATUS_ID))
                & (SYS_STATUS_RXFCG | SYS_STATUS_ALL_RX_ERR)));

        if(time == 0) {
            time = sys_timer_get_ms();
        }

        if (status_reg & SYS_STATUS_RXFCG) {
            // A frame has been received, copy it to our local buffer.
            frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
            if (frame_len <= FRAME_RECEIVE_LEN_MAX) {
                dwt_readrxdata(rx_buffer, frame_len, 0);
                totalFrame += frame_len;
                if(totalFrame == (1000 * 1000)) {
                    printf("Time: %u\n", (unsigned)sys_timer_get_elapsed_ms(time));
                    time = 0;
                    totalFrame = 0;
                }
                printf("Received %d bytes\n", frame_len);
            }

            // Clear good RX frame event in the DW1000 status register.
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXFCG);
        } else {
            printf("Error\n");
            // Clear RX error events in the DW1000 status register.
            dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_ERR);
            time = 0;
            totalFrame = 0;
        }
    }

    return 0;
}