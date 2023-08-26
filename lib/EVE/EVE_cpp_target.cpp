/*
@file    EVE_target.cpp
@brief   target specific functions for C++ targets, so far only Arduino targets
@version 5.0
@date    2023-05-31
@author  Rudolph Riedel

@section LICENSE

MIT License

Copyright (c) 2016-2023 Rudolph Riedel

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

@section History

5.0
- changed the DMA buffer from uin8_t to uint32_t
- added a section for Arduino-ESP32
- corrected the clock-divider settings for ESP32
- added DMA to ARDUINO_METRO_M4 target
- added DMA to ARDUINO_NUCLEO_F446RE target
- added DMA to Arduino-ESP32 target
- added a native ESP32 target with DMA
- added an ARDUINO_TEENSY41 target with DMA support
- added DMA for the Raspberry Pi Pico - RP2040
- added ARDUINO_TEENSY35 to the ARDUINO_TEENSY41 target
- split up this file in EVE_target.c for the plain C targets and EVE_target.cpp for the Arduino C++ targets
- converted all TABs to SPACEs
- copied over the SPI and DMA support functions for the RP2040 baremetall target to be used under Arduino
- modified the WIZIOPICO target for Arduino RP2040 to also work with ArduinoCore-mbed
- fixed the ESP32 target to work with the ESP32-S3 as well
- basic maintenance: checked for violations of white space and indent rules
- fixed a few warnings about missing initializers when compiling with the Arduino IDE 2.1.0

 */

#if defined (ARDUINO)

#if defined (ESP32)

/* note: this is using the ESP-IDF driver as the Arduino class and driver does not allow DMA for SPI */
#include "EVE_target.h"

spi_device_handle_t EVE_spi_device = {};
spi_device_handle_t EVE_spi_device_simple = {};

IRAM_ATTR static void eve_spi_post_transfer_callback(void)
{
    gpio_set_level(GPIO_NUM_10, 1);
    // digitalWrite(EVE_CS, HIGH); /* tell EVE to stop listen */
    #if defined (EVE_DMA)
        EVE_dma_busy = 0;
    #endif
}

IRAM_ATTR static void eve_spi_pre_transfer_callback(void)
{
    gpio_set_level(GPIO_NUM_10, 0);
    // digitalWrite(EVE_CS, LOW); /* make EVE listen */
    #if defined (EVE_DMA)
        EVE_dma_busy = 42;
    #endif
}

void EVE_init_spi(void)
{
    spi_bus_config_t buscfg = {};
    spi_device_interface_config_t devcfg = {};

    buscfg.mosi_io_num = EVE_MOSI;
    buscfg.miso_io_num = EVE_MISO;
    buscfg.sclk_io_num = EVE_SCK;
    buscfg.quadwp_io_num = EVE_GPIO0;//-1;
    buscfg.quadhd_io_num = EVE_GPIO1;//-1;
    buscfg.flags = SPICOMMON_BUSFLAG_QUAD;
    buscfg.max_transfer_sz = 640*8;

    devcfg.clock_speed_hz = SPI_MASTER_FREQ_26M; /* clock = 16 MHz */
    devcfg.flags = SPI_DEVICE_HALFDUPLEX;
    devcfg.mode = 0;                          /* SPI mode 0 */
    devcfg.spics_io_num = -1;                 /* CS pin operated by app */
    devcfg.queue_size = 4;                    /* we need only one transaction in the que */
    devcfg.address_bits = 24;                 /* 24 bits for the address */
    devcfg.command_bits = 0;                  /* command operated by app */
    devcfg.post_cb = (transaction_cb_t)eve_spi_post_transfer_callback;
    devcfg.pre_cb = (transaction_cb_t)eve_spi_pre_transfer_callback;

    spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO);
    spi_bus_add_device(SPI2_HOST, &devcfg, &EVE_spi_device);

    devcfg.spics_io_num = -1;                 /* CS pin operated by app */
    devcfg.address_bits = 0;
    devcfg.post_cb = 0;
    devcfg.pre_cb = 0;
    devcfg.clock_speed_hz = SPI_MASTER_FREQ_26M; /* Clock = 10 MHz */
    spi_bus_add_device(SPI2_HOST, &devcfg, &EVE_spi_device_simple);
}

#if defined (EVE_DMA)

uint32_t EVE_dma_buffer[1025U];
volatile uint16_t EVE_dma_buffer_index;
volatile uint8_t EVE_dma_busy = 0;

void EVE_init_dma(void)
{
}

void EVE_start_dma_transfer(void)
{
    spi_transaction_t EVE_spi_transaction = {};
    digitalWrite(EVE_CS, LOW); /* make EVE listen */
    EVE_spi_transaction.tx_buffer = (uint8_t *) &EVE_dma_buffer[1];
    EVE_spi_transaction.length = (EVE_dma_buffer_index-1) * 4U * 8U;
    EVE_spi_transaction.addr = 0x00b02578U; /* WRITE + REG_CMDB_WRITE; */
    spi_device_queue_trans(EVE_spi_device, &EVE_spi_transaction, portMAX_DELAY);
    EVE_dma_busy = 42;
}

#endif /* DMA */
#endif /* ESP32 */

#endif
