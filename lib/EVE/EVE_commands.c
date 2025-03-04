/*
@file    EVE_commands.c
@brief   contains FT8xx / BT8xx functions
@version 5.0
@date    2023-05-01
@author  Rudolph Riedel

@section info

At least for Arm Cortex-M0 and Cortex-M4 I have fastest execution with -O2.
The c-standard is C99.


@section LICENSE

MIT License

Copyright (c) 2016-2023 Rudolph Riedel

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


@section History

5.0
- added EVE_cmd_pclkfreq()
- put back writing of REG_CSSPREAD as it needs to be deactivated for higher frequencies
- added the configuration of the second PLL for the pixel clock in BT817/BT818 to EVE_init() in case the display config
has EVE_PCLK_FREQ defined
- replaced BT81X_ENABLE with "EVE_GEN > 2"
- removed FT81X_ENABLE as FT81x already is the lowest supported chip revision now
- removed the formerly as deprected marked EVE_get_touch_tag()
- changed EVE_color_rgb() to use a 32 bit value like the rest of the color commands
- removed the meta-commands EVE_cmd_point(), EVE_cmd_line() and EVE_cmd_rect()
- split all display-list commands into two functions: EVE_cmd_XXX() and EVE_cmd_XXX_burst()
- switched from using EVE_RAM_CMD + cmdOffset to REG_CMDB_WRITE
- as a side effect from switching to REG_CMDB_WRITE, every co-processor command is automatically executed now
- renamed EVE_LIB_GetProps() back to EVE_cmd_getprops() since it does not do anything special to justify a special name
- added helper function EVE_memWrite_sram_buffer()
- added EVE_cmd_bitmap_transform() and EVE_cmd_bitmap_transform_burst()
- added zero-pointer protection to commands using block_transfer()
- added EVE_cmd_playvideo()
- changed EVE_cmd_setfont() to a display-list command and added EVE_cmd_setfont_burst()
- changed EVE_cmd_setfont2() to a display-list command and added EVE_cmd_setfont2_burst()
- added EVE_cmd_videoframe()
- restructured: functions are sorted by chip-generation and within their group in alphabetical order
- reimplementedEVE_cmd_getmatrix() again, it needs to read values, not write them
- added EVE_cmd_fontcache() and EVE_cmd_fontcachequery()
- added EVE_cmd_calibratesub()
- added EVE_cmd_animframeram(), EVE_cmd_animframeram_burst(), EVE_cmd_animstartram(), EVE_cmd_animstartram_burst()
- added EVE_cmd_apilevel(), EVE_cmd_apilevel_burst()
- added EVE_cmd_calllist(), EVE_cmd_calllist_burst()
- added EVE_cmd_hsf(), EVE_cmd_hsf_burst()
- added EVE_cmd_linetime()
- added EVE_cmd_newlist(), EVE_cmd_newlist_burst()
- added EVE_cmd_runanim(), EVE_cmd_runanim_burst()
- added a safeguard to EVE_start_cmd_burst() to protect it from overlapping transfers with DMA and segmented lists
- used spi_transmit_32() to shorten this file by around 600 lines with no functional change
- removed the history from before 4.0
- removed a couple of spi_transmit_32() calls from EVE_cmd_getptr() to make it work again
- Bugfix: EVE_cmd_setfont2_burst() was using CMD_SETFONT instead of CMD_SETFONT2
- removed a check for cmd_burst from EVE_cmd_getimage() as it is in the group of commands that are not used for display
lists
- moved EVE_cmd_newlist() to the group of commands that are not used for display lists
- removed EVE_cmd_newlist_burst()
- renamed spi_flash_write() to private_block_write() and made it static
- renamed EVE_write_string() to private_string_write() and made it static
- made EVE_start_command() static
- Bugfix: ESP8266 needs 32 bit alignment for 32 bit pointers, changed private_string_write() for burst-mode to read
8-bit values
- Bugfix: somehow messed up private_string_write() for burst-mode but only for 8-Bit controllers
- changed EVE_memRead8(), EVE_memRead16() and EVE_memRead32() to use spi_transmit_32() for the initial address+zero byte
transfer This speeds up ESP32/ESP8266 by several µs, has no measureable effect for ATSAMD51 and is a little slower for
AVR.
- Bugfix: not sure why but setting private_block_write() to static broke it, without "static" it works
- Bugfix: EVE_cmd_flashspirx() was using CMD_FLASHREAD
- fixed a warning in EVE_init() when compiling for EVE4
- renamed internal function EVE_begin_cmd() to eve_begin_cmd() and made it static
- changed all the EVE_start_command() calls to eve_begin_cmd() calls following the report on Github from
  Michael Wachs that these are identical - they weren't prior to V5
- removed EVE_start_command()
- Bugfix: EVE_init() was only checking the first two bits of REG_CPURESET and ignored the bit for the audio-engine, not
an issue but not correct either.
- fixed a few clang-tidy warnings
- fixed a few cppcheck warnings
- fixed a few CERT warnings
- converted all TABs to SPACEs
- made EVE_TOUCH_RZTHRESH in EVE_init() optional to a) remove it from EVE_config.h and b) make it configureable
externally
- changed EVE_init() to write 1200U to REG_TOUCH_RZTHRESH if EVE_TOUCH_RZTHRESH is not defined
- changed EVE_init() to return E_OK = 0x00 in case of success and more meaningfull values in case of failure
- changed EVE_busy() to return EVE_IS_BUSY if EVE is busy and E_OK = 0x00 if EVE is not busy - no real change in
functionality
- finally removed EVE_cmd_start() after setting it to deprecatd with the first 5.0 release
- renamed EVE_cmd_execute() to EVE_execute_cmd() to be more consistent, this is is not an EVE command
- changed EVE_init_flash() to return E_OK in case of success and more meaningfull values in case of failure
- added the return-value of EVE_FIFO_HALF_EMPTY to EVE_busy() to indicate there is more than 2048 bytes available
- minor cleanup, less break and else statements
- added the burst code back into all the functions for which there is a _burst version, this allows to use the version
without the traling _burst in the name when exceution speed is not an issue - e.g. with all targets supporting DMA
- removed the 4.0 history
- added the optional parameter EVE_ROTATE as define to EVE_init() to allow for screen rotation during init
    thanks for the idea to AndrejValand on Github!
- added the optional parameter EVE_BACKLIGHT_PWM to EVE_init() to allow setting the backlight during init
- modified EVE_calibrate_manual() to work better with bar type displays
- fixed a large number of MISRA-C issues - mostly more casts for explicit type conversion and more brackets
- changed the varargs versions of cmd_button, cmd_text and cmd_toggle to use an array of uint32_t values to comply with MISRA-C
- basic maintenance: checked for violations of white space and indent rules
- more linter fixes for minor issues like variables shorter than 3 characters
- added EVE_color_a() / EVE_color_a_burst()
- more minor tweaks and fixes to make the static analyzer happy
- changed the burst variant of private_string_write() back to the older and faster version
- refactoring of EVE_init() to single return
- added prototype for EVE_write_display_parameters()
- added EVE_memRead_sram_buffer()
- Bugfix issue #81: neither DISP or the pixel clock are enabled for EVE4 configurations not using EVE_PCLK_FREQ.
    thanks for the report to grados73 on Github!
- added a few support lines for the Gameduino GD3X to EVE_init()
- switched from using CMD_PCLKFREQ to writing to REG_PCLK_FREQ directly
- added define EVE_SET_REG_PCLK_2X to set REG_PCLK_2X to 1 when necessary
- Bugfix: EVE_init() did not set the audio engine to "mute" as intended, but to "silent"
- Bugfix: EVE_busy() returns E_NOT_OK now on co-processor faults.
    thanks for the report to Z0ld3n on Github!
- Fix: reworked EVE_busy() to return EVE_FAULT_RECOVERED on deteced co-processor faults,
    removed the flash commands from the fault recovery sequence as these are project specific.
- added EVE_get_and_reset_fault_state() to check if EVE_busy() triggered a fault recovery
- added notes on how to use to EVE_cmd_setfont2() and EVE_cmd_romfont()

*/

#include "EVE.h"

/* EVE Memory Commands - used with EVE_memWritexx and EVE_memReadxx */
#define MEM_WRITE 0x80U /* EVE Host Memory Write */
/* #define MEM_READ 0x00U */ /* EVE Host Memory Read */

/* define NULL if it not already is */
#ifndef NULL
#include <stdio.h>
#endif

static volatile uint8_t cmd_burst = 0U; /* flag to indicate cmd-burst is active */
static volatile uint8_t fault_recovered = E_OK; /* flag to indicate if EVE_busy triggered a fault recovery */

/* ##################################################################
    helper functions
##################################################################### */


void EVE_cmdWrite(uint8_t command, uint8_t parameter)
{
    EVE_cs_set();
    spi_transmit(command);
    spi_transmit(parameter);
    spi_transmit(0U);
    EVE_cs_clear();
}

uint8_t EVE_memRead8(uint32_t ft_address)
{
    uint8_t data;
    EVE_cs_set();
    spi_transmit_32(((ft_address >> 16U) & 0x0000007fUL) + (ft_address & 0x0000ff00UL) + ((ft_address & 0x000000ffUL) << 16U));
    data = spi_receive(0U); /* read data byte by sending another dummy byte */
    EVE_cs_clear();
    return data;
}

uint16_t EVE_memRead16(uint32_t ft_address)
{
    uint16_t data;
    EVE_cs_set();
    spi_transmit_32(((ft_address >> 16U) & 0x0000007fUL) + (ft_address & 0x0000ff00UL) + ((ft_address & 0x000000ffUL) << 16U));
    data = ((uint16_t) spi_receive(0U)); /* read low byte */
    data = (((uint16_t) spi_receive(0U)) << 8U) | data; /* read high byte */
    EVE_cs_clear();
    return data;
}

uint32_t EVE_memRead32(uint32_t ft_address)
{
    uint32_t data;
    EVE_cs_set();
    spi_transmit_32(((ft_address >> 16U) & 0x0000007fUL) + (ft_address & 0x0000ff00UL) + ((ft_address & 0x000000ffUL) << 16U));
    data = ((uint32_t) spi_receive(0U)); /* read low byte */
    data = ((uint32_t) spi_receive(0U) << 8U) | data;
    data = ((uint32_t) spi_receive(0U) << 16U) | data;
    data = ((uint32_t) spi_receive(0U) << 24U) | data; /* read high byte */
    EVE_cs_clear();
    return data;
}

void EVE_memWrite8(uint32_t ft_address, uint8_t ft_data)
{
    EVE_cs_set();
    spi_transmit((uint8_t)(ft_address >> 16U) | MEM_WRITE);
    spi_transmit((uint8_t)(ft_address >> 8U));
    spi_transmit((uint8_t)(ft_address & 0x000000ffUL));
    spi_transmit(ft_data);
    EVE_cs_clear();
}

void EVE_memWrite16(uint32_t ft_address, uint16_t ft_data)
{
    EVE_cs_set();
    spi_transmit((uint8_t)(ft_address >> 16U) | MEM_WRITE); /* send Memory Write plus high address byte */
    spi_transmit((uint8_t)(ft_address >> 8U));              /* send middle address byte */
    spi_transmit((uint8_t)(ft_address & 0x000000ffUL));     /* send low address byte */
    spi_transmit((uint8_t)(ft_data & 0x00ffU));           /* send data low byte */
    spi_transmit((uint8_t)(ft_data >> 8U));               /* send data high byte */
    EVE_cs_clear();
}

void EVE_memWrite32(uint32_t ft_address, uint32_t ft_data)
{
    EVE_cs_set();
    spi_transmit((uint8_t)(ft_address >> 16U) | MEM_WRITE); /* send Memory Write plus high address byte */
    spi_transmit((uint8_t)(ft_address >> 8U));              /* send middle address byte */
    spi_transmit((uint8_t)(ft_address & 0x000000ffUL));     /* send low address byte */
    spi_transmit_32(ft_data);
    EVE_cs_clear();
}

/* Helper function, write a block of memory from the FLASH of the host controller to EVE. */
void EVE_memWrite_flash_buffer(uint32_t ft_address, const uint8_t *p_data, uint32_t len)
{
    if (p_data != NULL)
    {
        EVE_cs_set();
        spi_transmit((uint8_t)(ft_address >> 16U) | MEM_WRITE);
        spi_transmit((uint8_t)(ft_address >> 8U));
        spi_transmit((uint8_t)(ft_address & 0x000000ffUL));

    //    uint32_t length = (len + 3U) & (~3U);

        for (uint32_t count = 0U; count < len; count++)
        {
            spi_transmit(fetch_flash_byte(&p_data[count]));
        }

        EVE_cs_clear();
    }    
}

/* Helper function, write a block of memory from the SRAM of the host controller to EVE. */
void EVE_memWrite_sram_buffer(uint32_t ft_address, const uint8_t *p_data, uint32_t len)
{
    extern spi_device_handle_t EVE_spi_device;
    static spi_transaction_t EVE_spi_transaction[4] = {};
    static int index = 0, queued = 0;
    if (p_data != NULL && len > 0)
    {
        if (queued == 4)
        {
            spi_transaction_t* trans = NULL;
            spi_device_get_trans_result(EVE_spi_device, &trans, portMAX_DELAY);
            queued--;
        }
        EVE_spi_transaction[index].tx_buffer = p_data;
        EVE_spi_transaction[index].length = len * 8U;
        EVE_spi_transaction[index].addr = ft_address | (MEM_WRITE << 16U);
        EVE_spi_transaction[index].flags = SPI_TRANS_MODE_QIO|SPI_TRANS_MODE_DIOQIO_ADDR;
        spi_device_queue_trans(EVE_spi_device, &EVE_spi_transaction[index], portMAX_DELAY);
        index = (index + 1) & 3;
        queued++;
        //spi_device_transmit(EVE_spi_device, &EVE_spi_transaction);
        //while (EVE_dma_busy != 0) DELAY_MS(0);
    }
}

void EVE_switch_SPI(bool from1to4)
{
    if (from1to4)
        EVE_memWrite32(REG_SPI_WIDTH, 0x02U);
    else
    {
        //EVE_memWrite32(REG_SPI_WIDTH, 0x00U);
        extern spi_device_handle_t EVE_spi_device;
        spi_transaction_t EVE_spi_transaction = {};
        EVE_spi_transaction.tx_buffer = 0;
        EVE_spi_transaction.tx_data[0] = 0;
        EVE_spi_transaction.tx_data[1] = 0;
        EVE_spi_transaction.tx_data[2] = 0;
        EVE_spi_transaction.tx_data[3] = 0;
        EVE_spi_transaction.length = 32U;
        EVE_spi_transaction.addr = REG_SPI_WIDTH | (MEM_WRITE << 16U);
        EVE_spi_transaction.flags = SPI_TRANS_MODE_QIO|SPI_TRANS_MODE_DIOQIO_ADDR|SPI_TRANS_USE_TXDATA;
        digitalWrite(EVE_CS, LOW); /* make EVE listen */
        EVE_dma_busy = 42;
        spi_device_transmit(EVE_spi_device, &EVE_spi_transaction);
        //while (EVE_dma_busy != 0) DELAY_MS(0);

    }
}

void EVE_memWrite_sram_fill(uint32_t ft_address, const uint32_t data, uint32_t len)
{
    if (len > 0)
    {
        EVE_cs_set();
        spi_transmit((uint8_t)(ft_address >> 16U) | MEM_WRITE);
        spi_transmit((uint8_t)(ft_address >> 8U));
        spi_transmit((uint8_t)(ft_address & 0x000000ffUL));

        uint32_t length = (len + 3U) / 4;
        for (uint32_t count = 0U; count < length; count++)
        {
            spi_transmit_32(data);
        }

        EVE_cs_clear();
    }
}

/* Helper function, read a block of memory from EVE to the SRAM of the host controller */
void EVE_memRead_sram_buffer(uint32_t ft_address, uint8_t *p_data, uint32_t len)
{
    if (p_data != NULL)
    {
        EVE_cs_set();
        spi_transmit_32(((ft_address >> 16U) & 0x0000007fUL) + (ft_address & 0x0000ff00UL) + ((ft_address & 0x000000ffUL) << 16U));

        for (uint32_t count = 0U; count < len; count++)
        {
            p_data[count] = spi_receive(0U); /* read data byte by sending another dummy byte */
        }

        EVE_cs_clear();
    }    
}

static void CoprocessorFaultRecover(void)
{
#if EVE_GEN > 2
        uint16_t copro_patch_pointer;
        copro_patch_pointer = EVE_memRead16(REG_COPRO_PATCH_PTR);
#endif

        EVE_memWrite8(REG_CPURESET, 1U); /* hold co-processor engine in the reset condition */
        EVE_memWrite16(REG_CMD_READ, 0U); /* set REG_CMD_READ to 0 */
        EVE_memWrite16(REG_CMD_WRITE, 0U); /* set REG_CMD_WRITE to 0 */
        EVE_memWrite16(REG_CMD_DL, 0U); /* reset REG_CMD_DL to 0 as required by the BT81x programming guide, should not hurt FT8xx */

#if EVE_GEN > 2
        EVE_memWrite16(REG_COPRO_PATCH_PTR, copro_patch_pointer);

        /* restore REG_PCLK in case it was set to zero by an error */
#if (EVE_GEN > 3) && (defined EVE_PCLK_FREQ)
        EVE_memWrite16(REG_PCLK_FREQ, EVE_PCLK_FREQ);
        EVE_memWrite8(REG_PCLK, 1U); /* enable extsync mode */
#else
        EVE_memWrite8(REG_PCLK, EVE_PCLK);
#endif

#endif
        EVE_memWrite8(REG_CPURESET, 0U); /* set REG_CPURESET to 0 to restart the co-processor engine*/
        DELAY_MS(10U);                   /* just to be safe */
}

/* Check if the co-processor completed executing the current command list. */
/* Returns E_OK in case EVE is not busy (no DMA transfer active and REG_CMDB_SPACE has the value 0xffc, meaning the
 * CMD-FIFO is empty. */
/* If there was a coprocessor fault the recovery sequence is executed and E_NOT_OK is returned. */
/* note: in case of recovery the graphics context gets reset and the external flash needs to be reinitialized if needed */
/* Returns EVE_FIFO_HALF_EMPTY if no DMA transfer is active and REG_CMDB_SPACE shows more than 2048 bytes available. */
/* Returns EVE_IS_BUSY if a DMA transfer is active or REG_CMDB_SPACE has a value smaller than 0xffc. */
uint8_t EVE_busy(void)
{
    uint16_t space;
    uint8_t ret = EVE_IS_BUSY;

#if defined(EVE_DMA)
    if (0 == EVE_dma_busy)
    {
#endif

    space = EVE_memRead16(REG_CMDB_SPACE);

    /* (REG_CMDB_SPACE & 0x03) != 0 -> we have a co-processor fault */
    if ((space & 3U) != 0U) /* we have a co-processor fault, make EVE play with us again */
    {
        ret = EVE_FAULT_RECOVERED;
        fault_recovered = EVE_FAULT_RECOVERED; /* save fault recovery state */
        CoprocessorFaultRecover();
    }
    else
    {
        if (0xffcU == space)
        {
            ret = E_OK;
        }
        else if (space > 0x800U)
        {
            ret = EVE_FIFO_HALF_EMPTY;
        }
        else
        {
            ret = EVE_IS_BUSY;
        }
    }

#if defined(EVE_DMA)
    }
#endif

    return ret;
}

/* Returns EVE_FAULT_RECOVERED if EVE_busy() detected a coprocessor fault */
/* and tried to recover from it by resetting the coprocessor. */
/* The internal fault indicator is cleared so it could be set by EVE_busy() again. */
/* Returns E_OK if EVE_busy() did not detect a coprocessor fault. */
uint8_t EVE_get_and_reset_fault_state(void)
{
    uint8_t ret = E_OK;

    if (fault_recovered)
    {
        ret = EVE_FAULT_RECOVERED;
        fault_recovered = E_OK;
    }
    return ret;
}

/* Wait for the co-processor to complete the FIFO queue.*/
void EVE_execute_cmd(void)
{
    while (EVE_busy() != E_OK)
    {
    }
}

/* Begin a co-processor command, this is used for non-display-list and non-burst-mode commands.*/
static void eve_begin_cmd(uint32_t command)
{
    EVE_cs_set();
    spi_transmit((uint8_t) 0xB0U); /* high-byte of REG_CMDB_WRITE + MEM_WRITE */
    spi_transmit((uint8_t) 0x25U); /* middle-byte of REG_CMDB_WRITE */
    spi_transmit((uint8_t) 0x78U); /* low-byte of REG_CMDB_WRITE */
    spi_transmit_32(command);
}

void private_block_write(const uint8_t *p_data, uint16_t len); /* prototype to comply with MISRA */

void private_block_write(const uint8_t *p_data, uint16_t len)
{
    uint8_t padding;

    padding = (uint8_t) (len & 3U); /* 0, 1, 2 or 3 */
    padding = 4U - padding;         /* 4, 3, 2 or 1 */
    padding &= 3U;                  /* 3, 2 or 1 */

    for (uint16_t count = 0U; count < len; count++)
    {
        spi_transmit(fetch_flash_byte(&p_data[count]));
    }

    while (padding > 0U)
    {
        spi_transmit(0U);
        padding--;
    }
}

void block_transfer(const uint8_t *p_data, uint32_t len); /* prototype to comply with MISRA */

void block_transfer(const uint8_t *p_data, uint32_t len)
{
    uint32_t bytes_left;

    bytes_left = len;
    while (bytes_left > 0U)
    {
        uint32_t block_len;

        block_len = (bytes_left > 3840UL) ? 3840UL : bytes_left;

        EVE_cs_set();
        spi_transmit((uint8_t) 0xB0U); /* high-byte of REG_CMDB_WRITE + MEM_WRITE */
        spi_transmit((uint8_t) 0x25U); /* middle-byte of REG_CMDB_WRITE */
        spi_transmit((uint8_t) 0x78U); /* low-byte of REG_CMDB_WRITE */
        private_block_write(p_data, (uint16_t) block_len);
        EVE_cs_clear();
        p_data = &p_data[block_len];
        bytes_left -= block_len;
        EVE_execute_cmd();
    }
}

/* ##################################################################
    co-processor commands that are not used in displays lists,
    these are not to be used with burst transfers
################################################################### */

/* BT817 / BT818 */
#if EVE_GEN > 3

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
/* write "num" bytes from src in RAM_G to to the external flash on a BT81x board at address dest */
/* note: dest must be 4096-byte aligned, src must be 4-byte aligned, num must be a multiple of 4096 */
/* note: EVE will not do anything if the alignment requirements are not met */
/* note: the address ptr is relative to the flash so the first address is 0x00000000 not 0x800000 */
/* note: this looks exactly the same as EVE_cmd_flashupdate() but it needs the flash to be empty */
void EVE_cmd_flashprogram(uint32_t dest, uint32_t src, uint32_t num)
{
    eve_begin_cmd(CMD_FLASHPROGRAM);
    spi_transmit_32(dest);
    spi_transmit_32(src);
    spi_transmit_32(num);
    EVE_cs_clear();
    EVE_execute_cmd();
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
void EVE_cmd_fontcache(uint32_t font, int32_t ptr, uint32_t num)
{
    eve_begin_cmd(CMD_FONTCACHE);
    spi_transmit_32(font);
    spi_transmit_32((uint32_t) ptr);
    spi_transmit_32(num);
    EVE_cs_clear();
    EVE_execute_cmd();
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
void EVE_cmd_fontcachequery(uint32_t *p_total, int32_t *p_used)
{
    uint16_t cmdoffset;

    eve_begin_cmd(CMD_FONTCACHEQUERY);
    spi_transmit_32(0UL);
    spi_transmit_32(0UL);
    EVE_cs_clear();
    EVE_execute_cmd();

    cmdoffset = EVE_memRead16(REG_CMD_WRITE); /* read the co-processor write pointer */

    if (p_total != NULL)
    {
        *p_total = EVE_memRead32(EVE_RAM_CMD + ((cmdoffset - 8U) & 0xfffU));
    }
    if (p_used != NULL)
    {
        *p_used = (int32_t) EVE_memRead32(EVE_RAM_CMD + ((cmdoffset - 4U) & 0xfffU));
    }
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
void EVE_cmd_getimage(uint32_t *p_source, uint32_t *p_fmt, uint32_t *p_width, uint32_t *p_height, uint32_t *p_palette)
{
    uint16_t cmdoffset;

    eve_begin_cmd(CMD_GETIMAGE);
    spi_transmit_32(0UL);
    spi_transmit_32(0UL);
    spi_transmit_32(0UL);
    spi_transmit_32(0UL);
    spi_transmit_32(0UL);
    EVE_cs_clear();
    EVE_execute_cmd();

    cmdoffset = EVE_memRead16(REG_CMD_WRITE); /* read the co-processor write pointer */

    if (p_palette != NULL)
    {
        *p_palette = EVE_memRead32(EVE_RAM_CMD + ((cmdoffset - 4U) & 0xfffU));
    }
    if (p_height != NULL)
    {
        *p_height = EVE_memRead32(EVE_RAM_CMD + ((cmdoffset - 8U) & 0xfffU));
    }
    if (p_width != NULL)
    {
        *p_width = EVE_memRead32(EVE_RAM_CMD + ((cmdoffset - 12U) & 0xfffU));
    }
    if (p_fmt != NULL)
    {
        *p_fmt = EVE_memRead32(EVE_RAM_CMD + ((cmdoffset - 16U) & 0xfffU));
    }
    if (p_source != NULL)
    {
        *p_source = EVE_memRead32(EVE_RAM_CMD + ((cmdoffset - 20U) & 0xfffU));
    }
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
void EVE_cmd_linetime(uint32_t dest)
{
    eve_begin_cmd(CMD_LINETIME);
    spi_transmit_32(dest);
    EVE_cs_clear();
    EVE_execute_cmd();
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
void EVE_cmd_newlist(uint32_t adr)
{
    eve_begin_cmd(CMD_NEWLIST);
    spi_transmit_32(adr);
    EVE_cs_clear();
    EVE_execute_cmd();
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
/* This command sets REG_PCLK_FREQ to generate the closest possible frequency to the one requested. */
/* Returns the frequency achieved or zero if no frequency was found. */
uint32_t EVE_cmd_pclkfreq(uint32_t ftarget, int32_t rounding)
{
    uint16_t cmdoffset;

    eve_begin_cmd(CMD_PCLKFREQ);
    spi_transmit_32(ftarget);
    spi_transmit_32((uint32_t) rounding);
    spi_transmit_32(0UL);
    EVE_cs_clear();
    EVE_execute_cmd();
    cmdoffset = EVE_memRead16(REG_CMD_WRITE); /* read the co-processor write pointer */
    cmdoffset -= 4U;
    cmdoffset &= 0x0fffU;
    return (EVE_memRead32(EVE_RAM_CMD + cmdoffset));
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
void EVE_cmd_wait(uint32_t usec)
{
    eve_begin_cmd(CMD_WAIT);
    spi_transmit_32(usec);
    EVE_cs_clear();
    EVE_execute_cmd();
}

#endif /* EVE_GEN > 3 */

/* BT815 / BT816 */
#if EVE_GEN > 2

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
/* this command clears the graphics systems flash cache and to do so it needs to be executed with empty display lists */
/* note: looks like overkill to clear both display lists but this is taken from BRT sample code */
void EVE_cmd_clearcache(void)
{
    EVE_cmd_dl(CMD_DLSTART);
    EVE_cmd_dl(CMD_SWAP);
    EVE_execute_cmd();

    EVE_cmd_dl(CMD_DLSTART);
    EVE_cmd_dl(CMD_SWAP);
    EVE_execute_cmd();

    EVE_cmd_dl(CMD_CLEARCACHE);
    EVE_execute_cmd();
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
/* this is added for conveniance, using EVE_cmd_dl(CMD_FLASHATTACH); followed by EVE_execute_cmd(); would work as well
 */
void EVE_cmd_flashattach(void)
{
    eve_begin_cmd(CMD_FLASHATTACH);
    EVE_cs_clear();
    EVE_execute_cmd();
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
/* this is added for conveniance, using EVE_cmd_dl(CMD_FLASHDETACH); followed by EVE_execute_cmd(); would work as well
 */
void EVE_cmd_flashdetach(void)
{
    eve_begin_cmd(CMD_FLASHDETACH);
    EVE_cs_clear();
    EVE_execute_cmd();
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
/* this is added for conveniance, using EVE_cmd_dl(CMD_FLASHERASE); followed by EVE_execute_cmd(); would work as well */
void EVE_cmd_flasherase(void)
{
    eve_begin_cmd(CMD_FLASHERASE);
    EVE_cs_clear();
    EVE_execute_cmd();
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
uint32_t EVE_cmd_flashfast(void)
{
    uint16_t cmdoffset;

    eve_begin_cmd(CMD_FLASHFAST);
    spi_transmit_32(0UL);
    EVE_cs_clear();
    EVE_execute_cmd();
    cmdoffset = EVE_memRead16(REG_CMD_WRITE); /* read the co-processor write pointer */
    cmdoffset -= 4U;
    cmdoffset &= 0x0fffU;
    return (EVE_memRead32(EVE_RAM_CMD + cmdoffset));
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
/* this is added for conveniance, using EVE_cmd_dl(CMD_FLASHSPIDESEL); followed by EVE_execute_cmd(); would work as well
 */
void EVE_cmd_flashspidesel(void)
{
    eve_begin_cmd(CMD_FLASHSPIDESEL);
    EVE_cs_clear();
    EVE_execute_cmd();
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
/* write "num" bytes from src in the external flash on a BT81x board to dest in RAM_G */
/* note: src must be 64-byte aligned, dest must be 4-byte aligned, num must be a multiple of 4 */
/* note: EVE will not do anything if the alignment requirements are not met */
/* note: the src pointer is relative to the flash so the first address is 0x00000000 not 0x800000 */
void EVE_cmd_flashread(uint32_t dest, uint32_t src, uint32_t num)
{
    eve_begin_cmd(CMD_FLASHREAD);
    spi_transmit_32(dest);
    spi_transmit_32(src);
    spi_transmit_32(num);
    EVE_cs_clear();
    EVE_execute_cmd();
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
void EVE_cmd_flashsource(uint32_t ptr)
{
    eve_begin_cmd(CMD_FLASHSOURCE);
    spi_transmit_32(ptr);
    EVE_cs_clear();
    EVE_execute_cmd();
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
/* write "num" bytes from the BT81x SPI interface dest in RAM_G */
/* note: raw direct access, not really useful for anything */
void EVE_cmd_flashspirx(uint32_t dest, uint32_t num)
{
    eve_begin_cmd(CMD_FLASHSPIRX);
    spi_transmit_32(dest);
    spi_transmit_32(num);
    EVE_cs_clear();
    EVE_execute_cmd();
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
/* write "num" bytes from *p_data to the BT81x SPI interface */
/* note: raw direct access, not really useful for anything */
void EVE_cmd_flashspitx(uint32_t num, const uint8_t *p_data)
{
    eve_begin_cmd(CMD_FLASHSPITX);
    spi_transmit_32(num);
    EVE_cs_clear();
    block_transfer(p_data, num);
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
/* write "num" bytes from src in RAM_G to to the external flash on a BT81x board at address dest */
/* note: dest must be 4096-byte aligned, src must be 4-byte aligned, num must be a multiple of 4096 */
/* note: EVE will not do anything if the alignment requirements are not met */
/* note: the address ptr is relative to the flash so the first address is 0x00000000 not 0x800000 */
void EVE_cmd_flashupdate(uint32_t dest, uint32_t src, uint32_t num)
{
    eve_begin_cmd(CMD_FLASHUPDATE);
    spi_transmit_32(dest);
    spi_transmit_32(src);
    spi_transmit_32(num);
    EVE_cs_clear();
    EVE_execute_cmd();
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
/* write "num" bytes from *p_data to the external flash on a BT81x board at address ptr */
/* note: ptr must be 256 byte aligned, num must be a multiple of 256 */
/* note: EVE will not do anything if the alignment requirements are not met */
/* note: the address ptr is relative to the flash so the first address is 0x00000000 not 0x800000 */
/* note: on AVR controllers this expects the data to be located in the controllers flash memory */
void EVE_cmd_flashwrite(uint32_t ptr, uint32_t num, const uint8_t *p_data)
{
    eve_begin_cmd(CMD_FLASHWRITE);
    spi_transmit_32(ptr);
    spi_transmit_32(num);
    EVE_cs_clear();
    if (p_data != NULL)
    {
        block_transfer(p_data, num);
    }
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
void EVE_cmd_inflate2(uint32_t ptr, uint32_t options, const uint8_t *p_data, uint32_t len)
{
    eve_begin_cmd(CMD_INFLATE2);
    spi_transmit_32(ptr);
    spi_transmit_32(options);
    EVE_cs_clear();

    if (0UL == options) /* direct data, not by Media-FIFO or Flash */
    {
        if (p_data != NULL)
        {
            block_transfer(p_data, len);
        }
    }
}

#endif /* EVE_GEN > 2 */

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
/* get the properties of an image after a CMD_LOADIMAGE operation and write the values to the variables that are supplied by pointers*/
void EVE_cmd_getprops(uint32_t *p_pointer, uint32_t *p_width, uint32_t *p_height)
{
    uint16_t cmdoffset;

    eve_begin_cmd(CMD_GETPROPS);
    spi_transmit_32(0UL);
    spi_transmit_32(0UL);
    spi_transmit_32(0UL);
    EVE_cs_clear();
    EVE_execute_cmd();
    cmdoffset = EVE_memRead16(REG_CMD_WRITE); /* read the co-processor write pointer */

    if (p_pointer != NULL)
    {
        *p_pointer = EVE_memRead32(EVE_RAM_CMD + ((cmdoffset - 12U) & 0xfffU));
    }
    if (p_width != NULL)
    {
        *p_width = EVE_memRead32(EVE_RAM_CMD + ((cmdoffset - 8U) & 0xfffU));
    }
    if (p_height != NULL)
    {
        *p_height = EVE_memRead32(EVE_RAM_CMD + ((cmdoffset - 4U) & 0xfffU));
    }
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
uint32_t EVE_cmd_getptr(void)
{
    uint16_t cmdoffset;

    eve_begin_cmd(CMD_GETPTR);
    spi_transmit_32(0UL);
    EVE_cs_clear();
    EVE_execute_cmd();
    cmdoffset = EVE_memRead16(REG_CMD_WRITE); /* read the co-processor write pointer */
    cmdoffset -= 4U;
    cmdoffset &= 0x0fffU;
    return (EVE_memRead32(EVE_RAM_CMD + cmdoffset));
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
void EVE_cmd_inflate(uint32_t ptr, const uint8_t *p_data, uint32_t len)
{
    eve_begin_cmd(CMD_INFLATE);
    spi_transmit_32(ptr);
    EVE_cs_clear();
    if (p_data != NULL)
    {
        block_transfer(p_data, len);
    }
}

/* This is meant to be called outside display-list building, does not support cmd-burst.*/
void EVE_cmd_interrupt(uint32_t msec)
{
    eve_begin_cmd(CMD_INTERRUPT);
    spi_transmit_32(msec);
    EVE_cs_clear();
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
void EVE_cmd_loadimage(uint32_t ptr, uint32_t options, const uint8_t *p_data, uint32_t len)
{
    eve_begin_cmd(CMD_LOADIMAGE);
    spi_transmit_32(ptr);
    spi_transmit_32(options);
    EVE_cs_clear();

#if EVE_GEN > 2
    if ((0UL == (options & EVE_OPT_MEDIAFIFO)) &&
        (0UL == (options & EVE_OPT_FLASH))) /* direct data, neither by Media-FIFO or from Flash */
#else
    if (0UL == (options & EVE_OPT_MEDIAFIFO))  /* direct data, not by Media-FIFO */
#endif
    {
        if (p_data != NULL)
        {
            block_transfer(p_data, len);
        }
    }
}

/* This is meant to be called outside display-list building, does not support cmd-burst.*/
void EVE_cmd_mediafifo(uint32_t ptr, uint32_t size)
{
    eve_begin_cmd(CMD_MEDIAFIFO);
    spi_transmit_32(ptr);
    spi_transmit_32(size);
    EVE_cs_clear();
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
void EVE_cmd_memcpy(uint32_t dest, uint32_t src, uint32_t num)
{
    eve_begin_cmd(CMD_MEMCPY);
    spi_transmit_32(dest);
    spi_transmit_32(src);
    spi_transmit_32(num);
    EVE_cs_clear();
    EVE_execute_cmd();
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
uint32_t EVE_cmd_memcrc(uint32_t ptr, uint32_t num)
{
    uint16_t cmdoffset;

    eve_begin_cmd(CMD_MEMCRC);
    spi_transmit_32(ptr);
    spi_transmit_32(num);
    spi_transmit_32(0UL);
    EVE_cs_clear();
    EVE_execute_cmd();
    cmdoffset = EVE_memRead16(REG_CMD_WRITE); /* read the co-processor write pointer */
    cmdoffset -= 4U;
    cmdoffset &= 0x0fffU;
    return (EVE_memRead32(EVE_RAM_CMD + cmdoffset));
}

/* This is meant to be called outside display-list building, does not support cmd-burst.*/
void EVE_cmd_memset(uint32_t ptr, uint8_t value, uint32_t num)
{
    eve_begin_cmd(CMD_MEMSET);
    spi_transmit_32(ptr);
    spi_transmit_32((uint32_t)value);
    spi_transmit_32(num);
    EVE_cs_clear();
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
/* this is a pointless command, just use one of the EVE_memWrite* helper functions to directly write to EVEs memory */
/*
void EVE_cmd_memwrite(uint32_t dest, uint32_t num, const uint8_t *p_data)
{
    eve_begin_cmd(CMD_MEMWRITE);
    spi_transmit_32(dest);
    spi_transmit_32(num);

    num = (num + 3U) & (~3U);

    for (uint32_t count = 0U; count<len; count++)
    {
        spi_transmit(pgm_read_byte_far(p_data + count));
    }

    EVE_cs_clear();
    EVE_execute_cmd();
}
*/

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
void EVE_cmd_memzero(uint32_t ptr, uint32_t num)
{
    eve_begin_cmd(CMD_MEMZERO);
    spi_transmit_32(ptr);
    spi_transmit_32(num);
    EVE_cs_clear();
    EVE_execute_cmd();
}

/* This is meant to be called outside display-list building, it includes executing the command, does not support cmd-burst.*/
/* it does not wait for completion in order to allow the video to be paused or terminated by REG_PLAY_CONTROL */
void EVE_cmd_playvideo(uint32_t options, const uint8_t *p_data, uint32_t len)
{
    eve_begin_cmd(CMD_PLAYVIDEO);
    spi_transmit_32(options);
    EVE_cs_clear();

#if EVE_GEN > 2
    if ((0UL == (options & EVE_OPT_MEDIAFIFO)) &&
        (0UL == (options & EVE_OPT_FLASH))) /* direct data, neither by Media-FIFO or from Flash */
#else
    if (0UL == (options & EVE_OPT_MEDIAFIFO))  /* direct data, not by Media-FIFO */
#endif
    {
        if (p_data != NULL)
        {
            block_transfer(p_data, len);
        }
    }
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
/* regvalue = EVE_cmd_regread(ptr); */
/* this seems to be completely pointless, there is no real use for it outside a display-list since the register could be read directly */
/* and for what purpose would this be implemented to be used in a display list?? */
uint32_t EVE_cmd_regread(uint32_t ptr)
{
    uint16_t cmdoffset;

    eve_begin_cmd(CMD_REGREAD);
    spi_transmit_32(ptr);
    spi_transmit_32(0UL);
    EVE_cs_clear();
    EVE_execute_cmd();
    cmdoffset = EVE_memRead16(REG_CMD_WRITE); /* read the co-processor write pointer */
    cmdoffset -= 4U;
    cmdoffset &= 0x0fffU;
    return (EVE_memRead32(EVE_RAM_CMD + cmdoffset));
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
void EVE_cmd_setrotate(uint32_t rotation)
{
    eve_begin_cmd(CMD_SETROTATE);
    spi_transmit_32(rotation);
    EVE_cs_clear();
    EVE_execute_cmd();
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
void EVE_cmd_snapshot(uint32_t ptr)
{
    eve_begin_cmd(CMD_SNAPSHOT);
    spi_transmit_32(ptr);
    EVE_cs_clear();
    EVE_execute_cmd();
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
void EVE_cmd_snapshot2(uint32_t fmt, uint32_t ptr, int16_t xc0, int16_t yc0, int16_t wid, int16_t hgt)
{
    eve_begin_cmd(CMD_SNAPSHOT2);
    spi_transmit_32(fmt);
    spi_transmit_32(ptr);

    spi_transmit((uint8_t) ((uint16_t) xc0));
    spi_transmit((uint8_t) (((uint16_t) xc0) >> 8U));
    spi_transmit((uint8_t) ((uint16_t) yc0));
    spi_transmit((uint8_t) (((uint16_t) yc0) >> 8U));

    spi_transmit((uint8_t) ((uint16_t) wid));
    spi_transmit((uint8_t) (((uint16_t) wid) >> 8U));
    spi_transmit((uint8_t) ((uint16_t) hgt));
    spi_transmit((uint8_t) (((uint16_t) hgt) >> 8U));

    EVE_cs_clear();
    EVE_execute_cmd();
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
void EVE_cmd_track(int16_t xc0, int16_t yc0, int16_t wid, int16_t hgt, int16_t tag)
{
    eve_begin_cmd(CMD_TRACK);

    spi_transmit((uint8_t) ((uint16_t) xc0));
    spi_transmit((uint8_t) (((uint16_t) xc0) >> 8U));
    spi_transmit((uint8_t) ((uint16_t) yc0));
    spi_transmit((uint8_t) (((uint16_t) yc0) >> 8U));

    spi_transmit((uint8_t) ((uint16_t) wid));
    spi_transmit((uint8_t) (((uint16_t) wid) >> 8U));
    spi_transmit((uint8_t) ((uint16_t) hgt));
    spi_transmit((uint8_t) (((uint16_t) hgt) >> 8U));

    spi_transmit((uint8_t) ((uint16_t) tag));
    spi_transmit((uint8_t) (((uint16_t) tag) >> 8U));
    spi_transmit(0U);
    spi_transmit(0U);

    EVE_cs_clear();
    EVE_execute_cmd();
}

/* This is meant to be called outside display-list building, it includes executing the command and waiting for completion, does not support cmd-burst.*/
void EVE_cmd_videoframe(uint32_t dest, uint32_t result_ptr)
{
    eve_begin_cmd(CMD_VIDEOFRAME);
    spi_transmit_32(dest);
    spi_transmit_32(result_ptr);
    EVE_cs_clear();
    EVE_execute_cmd();
}

/* ##################################################################
        patching and initialization
#################################################################### */

#if EVE_GEN > 2

/**
 * @brief EVE flash initialization for BT81x, switches the FLASH attached to a BT81x to full-speed mode
 *
 * @return Returns E_OK in case of success, EVE_FAIL_FLASH_STATUS_INIT if the status remains init,
 * EVE_FAIL_FLASH_STATUS_DETACHED if no flash chip was found, a number of different values for failures with
 * cmd_flashfast and E_NOT_OK if a not supported status is returned in REG_FLASH_STATUS.
 */
uint8_t EVE_init_flash(void)
{
    uint8_t timeout = 0U;
    uint8_t status;
    uint8_t ret_val = E_NOT_OK;

    status = EVE_memRead8(REG_FLASH_STATUS); /* should be 0x02 - FLASH_STATUS_BASIC, power-up is done and the attached flash is detected */

     /* we are somehow still in init, give it a litte more time, this should never happen */
    while (EVE_FLASH_STATUS_INIT == status)
    {
        status = EVE_memRead8(REG_FLASH_STATUS);
        DELAY_MS(1U);
        timeout++;
        if (timeout > 100U) /* 100ms and still in init, lets call quits now and exit with an error */
        {
            ret_val = EVE_FAIL_FLASH_STATUS_INIT;
            break;
        }
    }

    /* no flash was found during init, no flash present or the detection failed, give it another try */
    if (EVE_FLASH_STATUS_DETACHED == status)
    {
        EVE_cmd_dl(CMD_FLASHATTACH);
        EVE_execute_cmd();
        status = EVE_memRead8(REG_FLASH_STATUS);
        if (status != 2U) /* still not in FLASH_STATUS_BASIC, time to give up */
        {
            ret_val = EVE_FAIL_FLASH_STATUS_DETACHED;
        }
    }

    /* flash detected and ready for action, move it up to FLASH_STATUS_FULL */
    if (EVE_FLASH_STATUS_BASIC == status)
    {
        uint32_t result;

        result = EVE_cmd_flashfast();

        switch (result)
        {
            case 0x0000UL:
                ret_val = E_OK;
                break;
            case 0xE001UL:
                ret_val = EVE_FAIL_FLASHFAST_NOT_SUPPORTED;
                break;
            case 0xE002UL:
                ret_val = EVE_FAIL_FLASHFAST_NO_HEADER_DETECTED;
                break;
            case 0xE003UL:
                ret_val = EVE_FAIL_FLASHFAST_SECTOR0_FAILED;
                break;
            case 0xE004UL:
                ret_val = EVE_FAIL_FLASHFAST_BLOB_MISMATCH;
                break;
            case 0xE005UL:
                ret_val = EVE_FAIL_FLASHFAST_SPEED_TEST;
                break;
            default: /* we have an unknown error, so just return failure */
                ret_val = E_NOT_OK;
                break;
        }
    }

    if (EVE_FLASH_STATUS_FULL == status) /* we are already there, why has this function been called? */
    {
        ret_val = E_OK;
    }

    return ret_val;
}

#endif /* EVE_GEN > 2 */

#if defined(EVE_HAS_GT911)

#if defined(__AVR__)
#include <avr/pgmspace.h>
#else
#define PROGMEM
#endif
void use_gt911(void);

void use_gt911(void)
{
#if EVE_GEN > 2
    EVE_memWrite16(REG_TOUCH_CONFIG, 0x05d0U); /* switch to Goodix touch controller */
#else

/* FT811 / FT813 binary-blob from FTDIs AN_336 to patch the touch-engine for Goodix GT911 / GT9271 touch controllers */
const uint8_t eve_gt911_data[1184U] PROGMEM = {
    26,  255, 255, 255, 32,  32,  48,  0,   4,   0,   0,   0,   2,   0,   0,   0,   34,  255, 255, 255, 0,   176, 48,
    0,   120, 218, 237, 84,  221, 111, 84,  69,  20,  63,  51,  179, 93,  160, 148, 101, 111, 76,  5,   44,  141, 123,
    111, 161, 11,  219, 154, 16,  9,   16,  17,  229, 156, 75,  26,  11,  13,  21,  227, 3,   16,  252, 184, 179, 45,
    219, 143, 45,  41,  125, 144, 72,  67,  100, 150, 71,  189, 113, 18,  36,  17,  165, 100, 165, 198, 16,  32,  17,
    149, 196, 240, 128, 161, 16,  164, 38,  54,  240, 0,   209, 72,  130, 15,  38,  125, 48,  66,  82,  30,  76,  19,
    31,  172, 103, 46,  139, 24,  255, 4,   227, 157, 204, 156, 51,  115, 102, 206, 231, 239, 220, 5,   170, 94,  129,
    137, 75,  194, 216, 98,  94,  103, 117, 115, 121, 76,  131, 177, 125, 89,  125, 82,  123, 60,  243, 58,  142, 242,
    204, 185, 243, 188, 118, 156, 227, 155, 203, 238, 238, 195, 251, 205, 229, 71,  92,  28,  169, 190, 184, 84,  143,
    113, 137, 53,  244, 103, 181, 237, 87,  253, 113, 137, 233, 48,  12,  198, 165, 181, 104, 139, 25,  84,  253, 155,
    114, 74,  191, 0,   54,  138, 163, 12,  62,  131, 207, 129, 23,  217, 34,  91,  31,  128, 65,  246, 163, 175, 213,
    8,   147, 213, 107, 35,  203, 94,  108, 3,   111, 40,  171, 83,  24,  15,  165, 177, 222, 116, 97,  23,  188, 140,
    206, 150, 42,  102, 181, 87,  78,  86,  182, 170, 134, 215, 241, 121, 26,  243, 252, 2,   76,  115, 217, 139, 222,
    206, 173, 136, 132, 81,  61,  35,  185, 39,  113, 23,  46,  199, 76,  178, 54,  151, 183, 224, 0,   40,  189, 28,
    149, 182, 58,  131, 79,  152, 30,  76,  34,  98,  234, 162, 216, 133, 141, 102, 39,  170, 40,  192, 101, 53,  201,
    146, 191, 37,  77,  44,  177, 209, 74,  211, 5,   206, 187, 5,   6,   216, 47,  53,  96,  123, 22,  50,  103, 251,
    192, 84,  17,  74,  227, 185, 56,  106, 51,  91,  161, 96,  182, 163, 48,  171, 141, 139, 65,  152, 66,  66,  11,
    102, 43,  158, 75,  36,  80,  147, 184, 147, 139, 112, 17,  235, 216, 103, 111, 239, 245, 92,  10,  175, 194, 40,
    44,  58,  125, 5,   59,  112, 50,  103, 245, 4,   78,  192, 5,   156, 194, 51,  60,  191, 134, 75,  110, 173, 237,
    46,  192, 121, 156, 192, 115, 184, 218, 120, 67,  63,  115, 46,  11,  102, 10,  97,  232, 50,  235, 114, 182, 148,
    118, 178, 41,  188, 12,  135, 77,  202, 124, 12,  96,  238, 35,  161, 234, 189, 129, 23,  249, 212, 139, 230, 25,
    53,  48,  205, 52,  93,  163, 117, 53,  154, 170, 81,  85,  163, 178, 70,  69,  66,  167, 241, 14,  46,  241, 1,
    226, 136, 152, 179, 197, 59,  184, 148, 254, 49,  132, 48,  15,  176, 137, 192, 76,  131, 196, 105, 104, 162, 86,
    81,  160, 165, 255, 26,  173, 162, 137, 86,  145, 210, 183, 192, 55,  175, 194, 211, 60,  91,  120, 230, 184, 174,
    27,  41,  131, 155, 40,  224, 29,  87,  179, 232, 16,  55,  55,  7,   165, 147, 81,  23,  165, 49,  101, 54,  224,
    75,  180, 81,  108, 18,  29,  226, 69,  225, 110, 175, 224, 42,  212, 25,  47,  130, 193, 110, 234, 192, 215, 252,
    56,  74,  162, 24,  46,  251, 174, 54,  106, 68,  245, 14,  9,   155, 160, 22,  120, 207, 104, 240, 29,  90,  178,
    140, 28,  24,  220, 47,  166, 112, 61,  251, 208, 192, 111, 56,  239, 238, 93,  255, 251, 62,  99,  32,  193, 75,
    61,  190, 235, 123, 229, 110, 218, 194, 85,  79,  225, 59,  98,  20,  238, 227, 235, 220, 11,  221, 149, 25,  180,
    116, 194, 159, 111, 96,  192, 24,  213, 59,  139, 179, 156, 215, 69,  230, 19,  24,  35,  135, 117, 206, 171, 206,
    162, 67,  129, 234, 61,  235, 11,  104, 103, 84,  64,  223, 167, 254, 40,  163, 101, 92,  84,  43,  150, 46,  249,
    219, 205, 7,   116, 11,  91,  104, 61,  57,  75,  223, 8,   48,  25,  28,  119, 252, 222, 113, 49,  86,  249, 74,
    180, 211, 156, 181, 61,  215, 168, 157, 7,   251, 199, 150, 242, 250, 91,  58,  132, 94,  121, 7,   53,  151, 139,
    98,  6,   165, 153, 69,  214, 32,  110, 211, 100, 101, 31,  89,  45,  81,  98,  23,  205, 205, 197, 209, 109, 186,
    198, 35,  141, 191, 249, 25,  60,  132, 223, 153, 251, 98,  20,  239, 146, 139, 20,  217, 250, 41,  250, 137, 58,
    177, 90,  57,  79,  51,  108, 233, 20,  253, 194, 187, 49,  222, 205, 114, 141, 96,  48,  175, 219, 107, 54,  111,
    138, 22,  154, 103, 108, 79,  58,  252, 179, 178, 79,  164, 195, 2,   153, 36,  39,  170, 199, 201, 167, 197, 85,
    106, 8,   59,  177, 81,  46,  56,  2,   230, 75,  114, 17,  55,  112, 188, 65,  208, 137, 77,  114, 10,  115, 55,
    58,  208, 197, 173, 122, 87,  6,   140, 110, 42,  208, 124, 163, 70,  108, 241, 104, 18,  245, 98,  214, 187, 134,
    53,  42,  221, 22,  182, 133, 211, 116, 148, 177, 194, 209, 192, 85,  90,  199, 58,  55,  203, 2,   229, 19,  137,
    187, 161, 228, 154, 112, 203, 145, 125, 244, 188, 220, 118, 228, 41,  201, 181, 41,  195, 144, 215, 183, 51,  80,
    250, 21,  217, 16,  217, 200, 235, 109, 227, 188, 122, 218, 142, 60,  170, 224, 112, 240, 184, 130, 229, 224, 113,
    5,   223, 148, 163, 80,  165, 183, 130, 187, 132, 116, 64,  238, 161, 85,  220, 115, 139, 205, 98,  227, 244, 29,
    102, 125, 7,   37,  243, 123, 223, 11,  26,  92,  63,  243, 116, 61,  191, 138, 123, 244, 160, 84,  186, 74,  31,
    5,   174, 247, 119, 135, 199, 248, 253, 135, 242, 97,  102, 145, 190, 144, 14,  85,  238, 221, 231, 193, 158, 48,
    205, 25,  120, 248, 15,  220, 29,  158, 9,   70,  185, 30,  103, 229, 33,  254, 23,  237, 160, 172, 62,  193, 90,
    222, 224, 232, 14,  200, 56,  90,  104, 142, 227, 120, 110, 6,   21,  211, 203, 65,  150, 99,  151, 220, 247, 87,
    164, 50,  159, 49,  239, 234, 58,  142, 0,   109, 108, 123, 18,  79,  227, 36,  100, 248, 222, 205, 96,  127, 120,
    26,  171, 228, 69,  63,  36,  17,  252, 200, 17,  116, 242, 187, 227, 88,  143, 247, 2,   75,  191, 6,   130, 59,
    188, 11,  55,  240, 31,  243, 122, 152, 226, 183, 207, 154, 73,  188, 39,  219, 43,  105, 222, 87,  41,  143, 141,
    140, 175, 73,  112, 184, 252, 61,  184, 16,  90,  250, 35,  168, 82,  119, 176, 57,  116, 94,  200, 150, 22,  190,
    179, 44,  104, 12,  235, 84,  149, 102, 252, 89,  154, 193, 99,  228, 106, 242, 125, 248, 64,  194, 255, 223, 127,
    242, 83,  11,  255, 2,   70,  214, 226, 128, 0,   0};

    EVE_cs_set();
    spi_transmit((uint8_t) 0xB0U); /* high-byte of REG_CMDB_WRITE + MEM_WRITE */
    spi_transmit((uint8_t) 0x25U); /* middle-byte of REG_CMDB_WRITE */
    spi_transmit((uint8_t) 0x78U); /* low-byte of REG_CMDB_WRITE */
    private_block_write(eve_gt911_data, sizeof(eve_gt911_data));
    EVE_cs_clear();
    EVE_execute_cmd();

    EVE_memWrite8(REG_TOUCH_OVERSAMPLE, 0x0fU); /* setup oversample to 0x0f as "hidden" in binary-blob for AN_336 */
    EVE_memWrite16(REG_TOUCH_CONFIG, 0x05D0U);  /* write magic cookie as requested by AN_336 */

    /* specific to the EVE2 modules from Matrix-Orbital we have to use GPIO3 to reset GT911 */
    EVE_memWrite16(REG_GPIOX_DIR, 0x8008U); /* Reset-Value is 0x8000, adding 0x08 sets GPIO3 to output, default-value
                                              for REG_GPIOX is 0x8000 -> Low output on GPIO3 */
    DELAY_MS(1U);                           /* wait more than 100�s */
    EVE_memWrite8(REG_CPURESET, 0U);        /* clear all resets */
    DELAY_MS(110U); /* wait more than 55ms - does not work with multitouch, for some reason a minimum delay of 108ms is
                      required */
    EVE_memWrite16(REG_GPIOX_DIR, 0x8000U); /* setting GPIO3 back to input */
#endif
}
#endif

static uint8_t wait_chipid(void)
{
    uint8_t ret = EVE_FAIL_CHIPID_TIMEOUT;
    uint8_t chipid = 0U;
    uint16_t timeout = 0U;

    for ( ; ; )
    {
        DELAY_MS(1U);

        chipid = EVE_memRead8(REG_ID);
        if(0x7cU == chipid) /* EVE is up and running */
        {
            ret = E_OK;
            break;
        }

        timeout++;
        if (timeout > 400U) /* EVE might not be with us at all */
        {
            break;
        }
    }

    return ret;
}

static uint8_t wait_reset(void)
{
    uint8_t ret = EVE_FAIL_RESET_TIMEOUT;
    uint8_t reset = 0U;
    uint16_t timeout = 0U;

    for ( ; ; )
    {
        DELAY_MS(1U);

        reset = EVE_memRead8(REG_CPURESET) & 7U;
        if(0U == reset) /* EVE reports all units running */
        {
            ret = E_OK;
            break;
        }

        timeout++;
        if (timeout > 50U) /* audio, touch or coprocessor engine fault */
        {
            break;
        }
    }

    return ret;
}

#if EVE_HPOL && EVE_VPOL
#define EVE_POL_GPIO_MASK 0x0C
#elif EVE_HPOL
#define EVE_POL_GPIO_MASK 0x04
#elif EVE_VPOL
#define EVE_POL_GPIO_MASK 0x08
#else
#define EVE_POL_GPIO_MASK 0x00
#endif

/* Writes all parameters defined for the display selected in EVE_config.h to */
/* the corresponding registers, is used by EVE_init() and can be used to */
/* refresh the register values if needed.*/
void EVE_write_display_parameters(void)
{
    /* Initialize Display */
    EVE_memWrite16(REG_HSIZE, EVE_HSIZE);     /* active display width */
    EVE_memWrite16(REG_HCYCLE, EVE_HCYCLE);   /* total number of clocks per line, incl front/back porch */
    EVE_memWrite16(REG_HOFFSET, EVE_HOFFSET); /* start of active line */
    EVE_memWrite16(REG_HSYNC0, EVE_HSYNC0);   /* start of horizontal sync pulse */
    EVE_memWrite16(REG_HSYNC1, EVE_HSYNC1);   /* end of horizontal sync pulse */
    EVE_memWrite16(REG_VSIZE, EVE_VSIZE);     /* active display height */
    EVE_memWrite16(REG_VCYCLE, EVE_VCYCLE);   /* total number of lines per screen, including pre/post */
    EVE_memWrite16(REG_VOFFSET, EVE_VOFFSET); /* start of active screen */
    EVE_memWrite16(REG_VSYNC0, EVE_VSYNC0);   /* start of vertical sync pulse */
    EVE_memWrite16(REG_VSYNC1, EVE_VSYNC1);   /* end of vertical sync pulse */
    EVE_memWrite8(REG_SWIZZLE, EVE_SWIZZLE);  /* FT8xx output to LCD - pin order */
    EVE_memWrite8(REG_PCLK_POL, EVE_PCLKPOL); /* LCD data is clocked in on this PCLK edge */
    EVE_memWrite8(REG_CSPREAD, EVE_CSPREAD); /* helps with noise, when set to 1 fewer signals are changed simultaneously, reset-default: 1 */

    EVE_memWrite16(REG_GPIOX_DIR, EVE_memRead16(REG_GPIOX_DIR) | EVE_POL_GPIO_MASK); // GPIO2/3 outputs
    EVE_memWrite16(REG_GPIOX, EVE_memRead16(REG_GPIOX) | EVE_POL_GPIO_MASK); // GPIO2/3 high

    /* configure Touch */
    EVE_memWrite8(REG_TOUCH_MODE, EVE_TMODE_CONTINUOUS); /* enable touch */
#if defined(EVE_TOUCH_RZTHRESH)
    EVE_memWrite16(REG_TOUCH_RZTHRESH, EVE_TOUCH_RZTHRESH); /* configure the sensitivity of resistive touch */
#else
    EVE_memWrite16(REG_TOUCH_RZTHRESH, 1200U); /* set a reasonable default value if none is given */
#endif

#if defined(EVE_ROTATE)
    EVE_memWrite8(REG_ROTATE, EVE_ROTATE & 7U); /* bit0 = invert, bit2 = portrait, bit3 = mirrored */
    /* reset default value is 0x0 - not inverted, landscape, not mirrored */
#endif
}

void enable_pixel_clock(void)
{
    EVE_memWrite8(REG_GPIO, 0x80U|0x10U); /* enable the DISP signal to the LCD panel, it is set to output in REG_GPIO_DIR by default */

#if (EVE_GEN > 3) && (defined EVE_PCLK_FREQ)
    EVE_memWrite16(REG_PCLK_FREQ, EVE_PCLK_FREQ);

#if defined (EVE_SET_REG_PCLK_2X)
    EVE_memWrite8(REG_PCLK_2X, 1U);
#endif

    EVE_memWrite8(REG_PCLK, 1U); /* enable extsync mode */
#else
    EVE_memWrite8(REG_PCLK, EVE_PCLK); /* start clocking data to the LCD panel */
#endif
}

/* EVE chip initialization, has to be executed with the SPI setup to 11 MHz or less as required by FT8xx / BT8xx! */
/* Takes no parameters but has a few optional parameters that are used by setting up defines. */
/* EVE_TOUCH_RZTHRESH - configure the sensitivity of resistive touch, defaults to 1200. */
/* EVE_ROTATE - set the screen rotation: bit0 = invert, bit2 = portrait, bit3 = mirrored */
/* Note: if you use this you need a set of calibration values for the selected rotation since this rotates before
 * calibration! */
/* EVE_BACKLIGHT_PWM - configure the backlight pwm, defaults to 0x20 / 25% */
/* Returns E_OK in case of success. */
uint8_t EVE_init(void)
{
    uint8_t ret;

    EVE_pdn_set();
    DELAY_MS(6U); /* minimum time for power-down is 5ms */
    EVE_pdn_clear();
    DELAY_MS(21U); /* minimum time to allow from rising PD_N to first access is 20ms */

#if defined(EVE_GD3X)
    EVE_cmdWrite(EVE_RST_PULSE,0U); /* reset, only required for warm-start if PowerDown line is not used */
#endif

#if defined(EVE_HAS_CRYSTAL)
    EVE_cmdWrite(EVE_CLKEXT, 0U); /* setup EVE for external clock */
#else
    EVE_cmdWrite(EVE_CLKINT, 0U); /* setup EVE for internal clock */
#endif

#if EVE_GEN > 2
    EVE_cmdWrite(EVE_CLKSEL, 0x46U); /* set clock to 72 MHz */
#endif

    EVE_cmdWrite(EVE_ACTIVE, 0U); /* start EVE */
    DELAY_MS(40U); /* give EVE a moment of silence to power up */

    ret = wait_chipid();
    if(E_OK == ret)
    {
        ret = wait_reset();
        if(E_OK == ret)
        {
/* tell EVE that we changed the frequency from default to 72MHz for BT8xx */
#if EVE_GEN > 2
            EVE_memWrite32(REG_FREQUENCY, 72000000UL);
#endif

/* we have a display with a Goodix GT911 / GT9271 touch-controller on it,
 so we patch our FT811 or FT813 according to AN_336 or setup a BT815 / BT817 accordingly */
#if defined(EVE_HAS_GT911)
            use_gt911();
#endif

#if defined(EVE_ADAM101)
            EVE_memWrite8(REG_PWM_DUTY, 0x80U); /* turn off backlight for Glyn ADAM101 module, it uses inverted values */
#else
            EVE_memWrite8(REG_PWM_DUTY, 0U); /* turn off backlight for any other module */
#endif
            EVE_write_display_parameters();

            /* disable Audio for now */
            EVE_memWrite8(REG_VOL_PB, 0U);      /* turn recorded audio volume down, reset-default is 0xff */
            EVE_memWrite8(REG_VOL_SOUND, 0U);   /* turn synthesizer volume down, reset-default is 0xff */
            EVE_memWrite16(REG_SOUND, EVE_MUTE); /* set synthesizer to mute */

            /* write a basic display-list to get things started */
            EVE_memWrite32(EVE_RAM_DL, DL_CLEAR_COLOR_RGB);
            EVE_memWrite32(EVE_RAM_DL + 4U, (DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG));
            EVE_memWrite32(EVE_RAM_DL + 8U, DL_DISPLAY); /* end of display list */
            EVE_memWrite32(REG_DLSWAP, EVE_DLSWAP_FRAME);
            /* nothing is being displayed yet... the pixel clock is still 0x00 */

#if defined (EVE_GD3X)
            EVE_memWrite16(REG_OUTBITS,0x01B6U); /* the GD3X is only using 6 bits per color */
#endif

            enable_pixel_clock();

#if defined(EVE_BACKLIGHT_PWM)
            EVE_memWrite8(REG_PWM_DUTY, EVE_BACKLIGHT_PWM); /* set backlight to user requested level */
#else
#if defined(EVE_ADAM101)
            EVE_memWrite8(REG_PWM_DUTY, 0x60U); /* turn on backlight to 25% for Glyn ADAM101 module, it uses inverted values */
#else
            EVE_memWrite8(REG_PWM_DUTY, 0x20U); /* turn on backlight to 25% for any other module */
#endif
#endif
            DELAY_MS(1U);
            EVE_execute_cmd(); /* just to be safe, wait for EVE to not be busy */

#if defined(EVE_DMA)
            EVE_init_dma(); /* prepare DMA */
#endif
        }
    }

    return ret;
}

/* ##################################################################
    functions for display lists
##################################################################### */

/* Begin a sequence of commands or prepare a DMA transfer if applicable. */
/* Needs to be used with EVE_end_cmd_burst(). */
/* Be careful to not use any functions in the sequence that do not address the command-fifo as for example any of EVE_mem...() functions. */
void EVE_start_cmd_burst(void)
{
#if defined(EVE_DMA)
    if (EVE_dma_busy)
    {
        EVE_execute_cmd(); /* this is a safe-guard to protect segmented display-list building with DMA from overlapping */
    }
#endif

    cmd_burst = 42U;

#if defined(EVE_DMA)
    EVE_dma_buffer[0U] = 0x7825B000UL; /* REG_CMDB_WRITE + MEM_WRITE low mid hi 00 */
//    ((uint8_t)(ft_address >> 16U) | MEM_WRITE) | (ft_address & 0x0000ff00UL) | ((uint8_t)(ft_address) << 16U);
//    EVE_dma_buffer[0U] = EVE_dma_buffer[0U] << 8U;
    EVE_dma_buffer_index = 1U;
#else
    EVE_cs_set();
    spi_transmit((uint8_t) 0xB0U); /* high-byte of REG_CMDB_WRITE + MEM_WRITE */
    spi_transmit((uint8_t) 0x25U); /* middle-byte of REG_CMDB_WRITE */
    spi_transmit((uint8_t) 0x78U); /* low-byte of REG_CMDB_WRITE */
#endif
}

/* End a sequence of commands or trigger a prepared DMA transfer if applicable. */
/* Needs to be used with EVE_start_cmd_burst(). */
void EVE_end_cmd_burst(void)
{
    cmd_burst = 0U;

#if defined(EVE_DMA)
    EVE_start_dma_transfer(); /* begin DMA transfer */
#else
    EVE_cs_clear();
#endif
}

#if 0
/* private function, begin a co-processor command, only used for non-burst commands */
static void EVE_start_command(uint32_t command)
{
    EVE_cs_set();
    spi_transmit((uint8_t) 0xB0U); /* high-byte of REG_CMDB_WRITE + MEM_WRITE */
    spi_transmit((uint8_t) 0x25U); /* middle-byte of REG_CMDB_WRITE */
    spi_transmit((uint8_t) 0x78U); /* low-byte of REG_CMDB_WRITE */
    spi_transmit_32(command);
}
#endif

/* write a string to co-processor memory in context of a command: */
/* no chip-select, just plain SPI-transfers */
static void private_string_write(const char *p_text)
{
    /* treat the array as bunch of bytes */
    const uint8_t *p_bytes = (const uint8_t *)p_text;

    if (0U == cmd_burst)
    {
        uint8_t textindex = 0U;
        uint8_t padding;

        /* either leave on Zero or when the string is too long */
        while ((textindex < 249U) && (p_bytes[textindex] != 0U))
        {
            spi_transmit(p_bytes[textindex]);
            textindex++;
        }

        /* transmit at least one 0x00 byte */
        /* and up to four if the string happens to be 4-byte aligned already */
        padding = textindex & 3U; /* 0, 1, 2 or 3 */
        padding = 4U - padding;   /* 4, 3, 2 or 1 */

        while (padding > 0U)
        {
            spi_transmit(0U);
            padding--;
        }
    }
    else
    {
        for(uint8_t textindex = 0U; textindex < 249U;)
        {
            uint32_t calc = 0U;
            uint8_t data;

            data = p_bytes[textindex++];
            if(0U == data)
            {
                spi_transmit_burst(calc);
                break;
            }
            calc += (uint32_t) (data);

            data = p_bytes[textindex++];
            if(0U == data)
            {
                spi_transmit_burst(calc);
                break;
            }
            calc += ((uint32_t) data) << 8U;

            data = p_bytes[textindex++];
            if(0U == data)
            {
                spi_transmit_burst(calc);
                break;
            }
            calc += ((uint32_t) data) << 16U;

            data = p_bytes[textindex++];
            if(0U == data)
            {
                spi_transmit_burst(calc);
                break;
            }
            calc += ((uint32_t) data) << 24U;

            spi_transmit_burst(calc);
        }
    }
}

/* BT817 / BT818 */
#if EVE_GEN > 3

void EVE_cmd_animframeram(int16_t xc0, int16_t yc0, uint32_t aoptr,
                            uint32_t frame)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_ANIMFRAMERAM);
        spi_transmit((uint8_t) ((uint16_t) xc0));
        spi_transmit((uint8_t) (((uint16_t) xc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) yc0));
        spi_transmit((uint8_t) (((uint16_t) yc0) >> 8U));
        spi_transmit_32(aoptr);
        spi_transmit_32(frame);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_ANIMFRAMERAM);
        spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
        spi_transmit_burst(aoptr);
        spi_transmit_burst(frame);
    }
}

void EVE_cmd_animframeram_burst(int16_t xc0, int16_t yc0, uint32_t aoptr,
                                uint32_t frame)
{
    spi_transmit_burst(CMD_ANIMFRAMERAM);
    spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
    spi_transmit_burst(aoptr);
    spi_transmit_burst(frame);
}

void EVE_cmd_animstartram(int32_t chnl, uint32_t aoptr, uint32_t loop)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_ANIMSTARTRAM);
        spi_transmit_32((uint32_t) chnl);
        spi_transmit_32(aoptr);
        spi_transmit_32(loop);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_ANIMSTARTRAM);
        spi_transmit_burst((uint32_t) chnl);
        spi_transmit_burst(aoptr);
        spi_transmit_burst(loop);
    }
}

void EVE_cmd_animstartram_burst(int32_t chnl, uint32_t aoptr, uint32_t loop)
{
    spi_transmit_burst(CMD_ANIMSTARTRAM);
    spi_transmit_burst((uint32_t) chnl);
    spi_transmit_burst(aoptr);
    spi_transmit_burst(loop);
}

void EVE_cmd_apilevel(uint32_t level)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_APILEVEL);
        spi_transmit_32(level);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_APILEVEL);
        spi_transmit_burst(level);
    }
}

void EVE_cmd_apilevel_burst(uint32_t level)
{
    spi_transmit_burst(CMD_APILEVEL);
    spi_transmit_burst(level);
}

void EVE_cmd_calibratesub(uint16_t xc0, uint16_t yc0, uint16_t width,
                            uint16_t height)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_CALIBRATESUB);
        spi_transmit((uint8_t) ((uint16_t) xc0));
        spi_transmit((uint8_t) (((uint16_t) xc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) yc0));
        spi_transmit((uint8_t) (((uint16_t) yc0) >> 8U));
        spi_transmit((uint8_t) (width));
        spi_transmit((uint8_t) (width >> 8U));
        spi_transmit((uint8_t) (height));
        spi_transmit((uint8_t) (height >> 8U));
        EVE_cs_clear();
    }
}

void EVE_cmd_calllist(uint32_t adr)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_CALLLIST);
        spi_transmit_32(adr);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_CALLLIST);
        spi_transmit_burst(adr);
    }
}

void EVE_cmd_calllist_burst(uint32_t adr)
{
    spi_transmit_burst(CMD_CALLLIST);
    spi_transmit_burst(adr);
}

void EVE_cmd_hsf(uint32_t hsf)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_HSF);
        spi_transmit_32(hsf);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_HSF);
        spi_transmit_burst(hsf);
    }
}

void EVE_cmd_hsf_burst(uint32_t hsf)
{
    spi_transmit_burst(CMD_HSF);
    spi_transmit_burst(hsf);
}

void EVE_cmd_runanim(uint32_t waitmask, uint32_t play)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_RUNANIM);
        spi_transmit_32(waitmask);
        spi_transmit_32(play);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_RUNANIM);
        spi_transmit_burst(waitmask);
        spi_transmit_burst(play);
    }
}

void EVE_cmd_runanim_burst(uint32_t waitmask, uint32_t play)
{
    spi_transmit_burst(CMD_RUNANIM);
    spi_transmit_burst(waitmask);
    spi_transmit_burst(play);
}

#endif /* EVE_GEN > 3 */

/* BT815 / BT816 */
#if EVE_GEN > 2

void EVE_cmd_animdraw(int32_t chnl)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_ANIMDRAW);
        spi_transmit_32((uint32_t) chnl);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_ANIMDRAW);
        spi_transmit_burst((uint32_t) chnl);
    }
}

void EVE_cmd_animdraw_burst(int32_t chnl)
{
    spi_transmit_burst(CMD_ANIMDRAW);
    spi_transmit_burst((uint32_t) chnl);
}

void EVE_cmd_animframe(int16_t xc0, int16_t yc0, uint32_t aoptr, uint32_t frame)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_ANIMFRAME);
        spi_transmit((uint8_t) ((uint16_t) xc0));
        spi_transmit((uint8_t) (((uint16_t) xc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) yc0));
        spi_transmit((uint8_t) (((uint16_t) yc0) >> 8U));
        spi_transmit_32(aoptr);
        spi_transmit_32(frame);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_ANIMFRAME);
        spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
        spi_transmit_burst(aoptr);
        spi_transmit_burst(frame);
    }
}

void EVE_cmd_animframe_burst(int16_t xc0, int16_t yc0, uint32_t aoptr,
                                uint32_t frame)
{
    spi_transmit_burst(CMD_ANIMFRAME);
    spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
    spi_transmit_burst(aoptr);
    spi_transmit_burst(frame);
}

void EVE_cmd_animstart(int32_t chnl, uint32_t aoptr, uint32_t loop)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_ANIMSTART);
        spi_transmit_32((uint32_t) chnl);
        spi_transmit_32(aoptr);
        spi_transmit_32(loop);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_ANIMSTART);
        spi_transmit_burst((uint32_t) chnl);
        spi_transmit_burst(aoptr);
        spi_transmit_burst(loop);
    }
}

void EVE_cmd_animstart_burst(int32_t chnl, uint32_t aoptr, uint32_t loop)
{
    spi_transmit_burst(CMD_ANIMSTART);
    spi_transmit_burst((uint32_t) chnl);
    spi_transmit_burst(aoptr);
    spi_transmit_burst(loop);
}

void EVE_cmd_animstop(int32_t chnl)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_ANIMSTOP);
        spi_transmit_32((uint32_t) chnl);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_ANIMSTOP);
        spi_transmit_burst((uint32_t) chnl);
    }
}

void EVE_cmd_animstop_burst(int32_t chnl)
{
    spi_transmit_burst(CMD_ANIMSTOP);
    spi_transmit_burst((uint32_t) chnl);
}

void EVE_cmd_animxy(int32_t chnl, int16_t xc0, int16_t yc0)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_ANIMXY);
        spi_transmit_32((uint32_t) chnl);
        spi_transmit((uint8_t) ((uint16_t) xc0));
        spi_transmit((uint8_t) (((uint16_t) xc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) yc0));
        spi_transmit((uint8_t) (((uint16_t) yc0) >> 8U));
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_ANIMXY);
        spi_transmit_burst((uint32_t) chnl);
        spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
    }
}

void EVE_cmd_animxy_burst(int32_t chnl, int16_t xc0, int16_t yc0)
{
    spi_transmit_burst(CMD_ANIMXY);
    spi_transmit_burst((uint32_t) chnl);
    spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
}

void EVE_cmd_appendf(uint32_t ptr, uint32_t num)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_APPENDF);
        spi_transmit_32(ptr);
        spi_transmit_32(num);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_APPENDF);
        spi_transmit_burst(ptr);
        spi_transmit_burst(num);
    }
}

void EVE_cmd_appendf_burst(uint32_t ptr, uint32_t num)
{
    spi_transmit_burst(CMD_APPENDF);
    spi_transmit_burst(ptr);
    spi_transmit_burst(num);
}

uint16_t EVE_cmd_bitmap_transform(int32_t xc0, int32_t yc0, int32_t xc1,
                                int32_t yc1, int32_t xc2, int32_t yc2,
                                int32_t tx0, int32_t ty0, int32_t tx1,
                                int32_t ty1, int32_t tx2, int32_t ty2)
{
    uint16_t ret_val = 0U;

    if (0U == cmd_burst)
    {
        uint16_t cmdoffset;

        eve_begin_cmd(CMD_BITMAP_TRANSFORM);
        spi_transmit_32((uint32_t) xc0);
        spi_transmit_32((uint32_t) yc0);
        spi_transmit_32((uint32_t) xc1);
        spi_transmit_32((uint32_t) yc1);
        spi_transmit_32((uint32_t) xc2);
        spi_transmit_32((uint32_t) yc2);
        spi_transmit_32((uint32_t) tx0);
        spi_transmit_32((uint32_t) ty0);
        spi_transmit_32((uint32_t) tx1);
        spi_transmit_32((uint32_t) ty1);
        spi_transmit_32((uint32_t) tx2);
        spi_transmit_32((uint32_t) ty2);
        spi_transmit_32(0UL);
        EVE_cs_clear();
        EVE_execute_cmd();
        cmdoffset = EVE_memRead16(REG_CMD_WRITE);
        cmdoffset -= 4U;
        cmdoffset &= 0x0fffU;
        ret_val = (uint16_t) EVE_memRead32(EVE_RAM_CMD + cmdoffset);
    }
    else /* note: the result parameter is ignored in burst mode */
    {
        spi_transmit_burst(CMD_BITMAP_TRANSFORM);
        spi_transmit_burst((uint32_t) xc0);
        spi_transmit_burst((uint32_t) yc0);
        spi_transmit_burst((uint32_t) xc1);
        spi_transmit_burst((uint32_t) yc1);
        spi_transmit_burst((uint32_t) xc2);
        spi_transmit_burst((uint32_t) yc2);
        spi_transmit_burst((uint32_t) tx0);
        spi_transmit_burst((uint32_t) ty0);
        spi_transmit_burst((uint32_t) tx1);
        spi_transmit_burst((uint32_t) ty1);
        spi_transmit_burst((uint32_t) tx2);
        spi_transmit_burst((uint32_t) ty2);
        spi_transmit_burst(0UL);
    }
    return ret_val;
}

/* note: as this is meant for use in burst-mode display-list generation */
/* the result parameter is ignored */
void EVE_cmd_bitmap_transform_burst(int32_t xc0, int32_t yc0, int32_t xc1,
                                int32_t yc1, int32_t xc2, int32_t yc2,
                                int32_t tx0, int32_t ty0, int32_t tx1,
                                int32_t ty1, int32_t tx2, int32_t ty2)
{
    spi_transmit_burst(CMD_BITMAP_TRANSFORM);
    spi_transmit_burst((uint32_t) xc0);
    spi_transmit_burst((uint32_t) yc0);
    spi_transmit_burst((uint32_t) xc1);
    spi_transmit_burst((uint32_t) yc1);
    spi_transmit_burst((uint32_t) xc2);
    spi_transmit_burst((uint32_t) yc2);
    spi_transmit_burst((uint32_t) tx0);
    spi_transmit_burst((uint32_t) ty0);
    spi_transmit_burst((uint32_t) tx1);
    spi_transmit_burst((uint32_t) ty1);
    spi_transmit_burst((uint32_t) tx2);
    spi_transmit_burst((uint32_t) ty2);
    spi_transmit_burst(0UL);
}

void EVE_cmd_fillwidth(uint32_t pixel)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_FILLWIDTH);
        spi_transmit_32(pixel);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_FILLWIDTH);
        spi_transmit_burst(pixel);
    }
}

void EVE_cmd_fillwidth_burst(uint32_t pixel)
{
    spi_transmit_burst(CMD_FILLWIDTH);
    spi_transmit_burst(pixel);
}

void EVE_cmd_gradienta(int16_t xc0, int16_t yc0, uint32_t argb0, int16_t xc1,
                        int16_t yc1, uint32_t argb1)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_GRADIENTA);
        spi_transmit((uint8_t) ((uint16_t) xc0));
        spi_transmit((uint8_t) (((uint16_t) xc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) yc0));
        spi_transmit((uint8_t) (((uint16_t) yc0) >> 8U));
        spi_transmit_32(argb0);
        spi_transmit((uint8_t) ((uint16_t) xc1));
        spi_transmit((uint8_t) (((uint16_t) xc1) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) yc1));
        spi_transmit((uint8_t) (((uint16_t) yc1) >> 8U));
        spi_transmit_32(argb1);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_GRADIENTA);
        spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
        spi_transmit_burst(argb0);
        spi_transmit_burst(((uint32_t) ((uint16_t) xc1)) + (((uint32_t) ((uint16_t) yc1)) << 16U));
        spi_transmit_burst(argb1);
    }
}

void EVE_cmd_gradienta_burst(int16_t xc0, int16_t yc0, uint32_t argb0,
                                int16_t xc1, int16_t yc1, uint32_t argb1)
{
    spi_transmit_burst(CMD_GRADIENTA);
    spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
    spi_transmit_burst(argb0);
    spi_transmit_burst(((uint32_t) ((uint16_t) xc1)) + (((uint32_t) ((uint16_t) yc1)) << 16U));
    spi_transmit_burst(argb1);
}

void EVE_cmd_rotatearound(int32_t xc0, int32_t yc0, int32_t angle, int32_t scale)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_ROTATEAROUND);
        spi_transmit_32((uint32_t) xc0);
        spi_transmit_32((uint32_t) yc0);
        spi_transmit_32((uint32_t) angle);
        spi_transmit_32((uint32_t) scale);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_ROTATEAROUND);
        spi_transmit_burst((uint32_t) xc0);
        spi_transmit_burst((uint32_t) yc0);
        spi_transmit_burst((uint32_t) angle);
        spi_transmit_burst((uint32_t) scale);
    }
}

void EVE_cmd_rotatearound_burst(int32_t xc0, int32_t yc0, int32_t angle,
                                int32_t scale)
{
    spi_transmit_burst(CMD_ROTATEAROUND);
    spi_transmit_burst((uint32_t) xc0);
    spi_transmit_burst((uint32_t) yc0);
    spi_transmit_burst((uint32_t) angle);
    spi_transmit_burst((uint32_t) scale);
}

/* "num_args" is the number of elements provided in "p_arguments[]" */
void EVE_cmd_button_var(int16_t xc0, int16_t yc0, int16_t wid, int16_t hgt,
                        int16_t font, uint16_t options, const char *p_text,
                        uint8_t num_args, const uint32_t p_arguments[])
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_BUTTON);
        spi_transmit((uint8_t) ((uint16_t) xc0));
        spi_transmit((uint8_t) (((uint16_t) xc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) yc0));
        spi_transmit((uint8_t) (((uint16_t) yc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) wid));
        spi_transmit((uint8_t) (((uint16_t) wid) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) hgt));
        spi_transmit((uint8_t) (((uint16_t) hgt) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) font));
        spi_transmit((uint8_t) (((uint16_t) font) >> 8U));
        spi_transmit((uint8_t) (options));
        spi_transmit((uint8_t) (options >> 8U));
        private_string_write(p_text);

        if ((options & EVE_OPT_FORMAT) != 0U)
        {
            if (p_arguments != NULL)
            {
                for (uint8_t counter = 0U; counter < num_args; counter++)
                {
                    spi_transmit_32(p_arguments[counter]);
                }
            }
        }
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_BUTTON);
        spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
        spi_transmit_burst(((uint32_t) ((uint16_t) wid)) + (((uint32_t) ((uint16_t) hgt)) << 16U));
        spi_transmit_burst(((uint32_t) ((uint16_t) font)) + ((uint32_t) options << 16U));
        private_string_write(p_text);

        if ((options & EVE_OPT_FORMAT) != 0U)
        {
            if (p_arguments != NULL)
            {
                for (uint8_t counter = 0U; counter < num_args; counter++)
                {
                    spi_transmit_burst(p_arguments[counter]);
                }
            }
        }
    }
}

/* "num_args" is the number of elements provided in "p_arguments[]" */
void EVE_cmd_button_var_burst(int16_t xc0, int16_t yc0, int16_t wid, int16_t hgt,
                              int16_t font, uint16_t options, const char *p_text,
                              uint8_t num_args, const uint32_t p_arguments[])
{
    spi_transmit_burst(CMD_BUTTON);
    spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
    spi_transmit_burst(((uint32_t) ((uint16_t) wid)) + (((uint32_t) ((uint16_t) hgt)) << 16U));
    spi_transmit_burst(((uint32_t) ((uint16_t) font)) + ((uint32_t) options << 16U));
    private_string_write(p_text);

    if ((options & EVE_OPT_FORMAT) != 0U)
    {
        if (p_arguments != NULL)
        {
            for (uint8_t counter = 0U; counter < num_args; counter++)
            {
                spi_transmit_burst(p_arguments[counter]);
            }
        }
    }
}

/* "num_args" is the number of elements provided in "p_arguments[]" */
void EVE_cmd_text_var(int16_t xc0, int16_t yc0, int16_t font,
                        uint16_t options, const char *p_text,
                        uint8_t num_args, const uint32_t p_arguments[])
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_TEXT);
        spi_transmit((uint8_t) ((uint16_t) xc0));
        spi_transmit((uint8_t) (((uint16_t) xc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) yc0));
        spi_transmit((uint8_t) (((uint16_t) yc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) font));
        spi_transmit((uint8_t) (((uint16_t) font) >> 8U));
        spi_transmit((uint8_t) (options));
        spi_transmit((uint8_t) (options >> 8U));
        private_string_write(p_text);

        if ((options & EVE_OPT_FORMAT) != 0U)
        {
            if (p_arguments != NULL)
            {
                for (uint8_t counter = 0U; counter < num_args; counter++)
                {
                    spi_transmit_32(p_arguments[counter]);
                }
            }
        }
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_TEXT);
        spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
        spi_transmit_burst(((uint32_t) ((uint16_t) font)) + (((uint32_t) options) << 16U));
        private_string_write(p_text);

        if ((options & EVE_OPT_FORMAT) != 0U)
        {
            if (p_arguments != NULL)
            {
                for (uint8_t counter = 0U; counter < num_args; counter++)
                {
                    spi_transmit_burst(p_arguments[counter]);
                }
            }
        }
    }
}

/* "num_args" is the number of elements provided in "p_arguments[]" */
void EVE_cmd_text_var_burst(int16_t xc0, int16_t yc0, int16_t font,
                            uint16_t options, const char *p_text,
                            uint8_t num_args, const uint32_t p_arguments[])
{
    spi_transmit_burst(CMD_TEXT);
    spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
    spi_transmit_burst(((uint32_t) ((uint16_t) font)) + (((uint32_t) options) << 16U));
    private_string_write(p_text);

    if ((options & EVE_OPT_FORMAT) != 0U)
    {
        if (p_arguments != NULL)
        {
            for (uint8_t counter = 0U; counter < num_args; counter++)
            {
                spi_transmit_burst(p_arguments[counter]);
            }
        }
    }
}

/* "num_args" is the number of elements provided in "p_arguments[]" */
void EVE_cmd_toggle_var(int16_t xc0, int16_t yc0, int16_t wid, int16_t font,
                        uint16_t options, uint16_t state, const char *p_text,
                        uint8_t num_args, const uint32_t p_arguments[])
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_TOGGLE);
        spi_transmit((uint8_t) ((uint16_t) xc0));
        spi_transmit((uint8_t) (((uint16_t) xc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) yc0));
        spi_transmit((uint8_t) (((uint16_t) yc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) wid));
        spi_transmit((uint8_t) (((uint16_t) wid) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) font));
        spi_transmit((uint8_t) (((uint16_t) font) >> 8U));
        spi_transmit((uint8_t) (options));
        spi_transmit((uint8_t) (options >> 8U));
        spi_transmit((uint8_t) (state));
        spi_transmit((uint8_t) (state >> 8U));
        private_string_write(p_text);

        if ((options & EVE_OPT_FORMAT) != 0U)
        {
            if (p_arguments != NULL)
            {
                for (uint8_t counter = 0U; counter < num_args; counter++)
                {
                    spi_transmit_32(p_arguments[counter]);
                }
            }
        }
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_TOGGLE);
        spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
        spi_transmit_burst(((uint32_t) ((uint16_t) wid)) + (((uint32_t) ((uint16_t) font) << 16U)));
        spi_transmit_burst((uint32_t) options + (((uint32_t) state) << 16U));
        private_string_write(p_text);

        if ((options & EVE_OPT_FORMAT) != 0U)
        {
            if (p_arguments != NULL)
            {
                for (uint8_t counter = 0U; counter < num_args; counter++)
                {
                    spi_transmit_burst(p_arguments[counter]);
                }
            }
        }
    }
}

/* "num_args" is the number of elements provided in "p_arguments[]" */
void EVE_cmd_toggle_var_burst(int16_t xc0, int16_t yc0, int16_t wid, int16_t font,
                            uint16_t options, uint16_t state, const char *p_text,
                            uint8_t num_args, const uint32_t p_arguments[])
{
    spi_transmit_burst(CMD_TOGGLE);
    spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
    spi_transmit_burst(((uint32_t) ((uint16_t) wid)) + (((uint32_t) ((uint16_t) font) << 16U)));
    spi_transmit_burst((uint32_t) options + (((uint32_t) state) << 16U));
    private_string_write(p_text);

    if ((options & EVE_OPT_FORMAT) != 0U)
    {
        if (p_arguments != NULL)
        {
            for (uint8_t counter = 0U; counter < num_args; counter++)
            {
                spi_transmit_burst(p_arguments[counter]);
            }
        }
    }
}

#endif /* EVE_GEN > 2 */

/* generic function for: */
/* all co-processor commands that have no arguments */
/* all display-list commands */
/*
 examples:
 EVE_cmd_dl(CMD_DLSTART);
 EVE_cmd_dl(CMD_SWAP);
 EVE_cmd_dl(CMD_SCREENSAVER);
 EVE_cmd_dl(LINE_WIDTH(1*16));
 EVE_cmd_dl(VERTEX2F(0,0));
 EVE_cmd_dl(DL_BEGIN | EVE_RECTS);
*/
void EVE_cmd_dl(uint32_t command)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(command);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(command);
    }
}

void EVE_cmd_dl_burst(uint32_t command)
{
    spi_transmit_burst(command);
}

void EVE_cmd_append(uint32_t ptr, uint32_t num)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_APPEND);
        spi_transmit_32(ptr);
        spi_transmit_32(num);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_APPEND);
        spi_transmit_burst(ptr);
        spi_transmit_burst(num);
    }
}

void EVE_cmd_append_burst(uint32_t ptr, uint32_t num)
{
    spi_transmit_burst(CMD_APPEND);
    spi_transmit_burst(ptr);
    spi_transmit_burst(num);
}

void EVE_cmd_bgcolor(uint32_t color)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_BGCOLOR);
        spi_transmit((uint8_t) (color));
        spi_transmit((uint8_t) (color >> 8U));
        spi_transmit((uint8_t) (color >> 16U));
        spi_transmit(0U);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_BGCOLOR);
        spi_transmit_burst(color);
    }
}

void EVE_cmd_bgcolor_burst(uint32_t color)
{
    spi_transmit_burst(CMD_BGCOLOR);
    spi_transmit_burst(color);
}

void EVE_cmd_button(int16_t xc0, int16_t yc0, int16_t wid, int16_t hgt,
                    int16_t font, uint16_t options, const char *p_text)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_BUTTON);
        spi_transmit((uint8_t) ((uint16_t) xc0));
        spi_transmit((uint8_t) (((uint16_t) xc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) yc0));
        spi_transmit((uint8_t) (((uint16_t) yc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) wid));
        spi_transmit((uint8_t) (((uint16_t) wid) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) hgt));
        spi_transmit((uint8_t) (((uint16_t) hgt) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) font));
        spi_transmit((uint8_t) (((uint16_t) font) >> 8U));
        spi_transmit((uint8_t) (options));
        spi_transmit((uint8_t) (options >> 8U));
        private_string_write(p_text);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_BUTTON);
        spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
        spi_transmit_burst(((uint32_t) ((uint16_t) wid)) + (((uint32_t) ((uint16_t) hgt)) << 16U));
        spi_transmit_burst(((uint32_t) ((uint16_t) font)) + (((uint32_t) options) << 16U));
        private_string_write(p_text);
    }
}

void EVE_cmd_button_burst(int16_t xc0, int16_t yc0, int16_t wid, int16_t hgt,
                            int16_t font, uint16_t options, const char *p_text)
{
    spi_transmit_burst(CMD_BUTTON);
    spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
    spi_transmit_burst(((uint32_t) ((uint16_t) wid)) + (((uint32_t) ((uint16_t) hgt)) << 16U));
    spi_transmit_burst(((uint32_t) ((uint16_t) font)) + (((uint32_t) options) << 16U));
    private_string_write(p_text);
}

void EVE_cmd_calibrate(void)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_CALIBRATE);
        spi_transmit_32(0UL);
        EVE_cs_clear();
    }
}

void EVE_cmd_clock(int16_t xc0, int16_t yc0, int16_t rad, uint16_t options, uint16_t hours,
                    uint16_t mins, uint16_t secs, uint16_t msecs)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_CLOCK);
        spi_transmit((uint8_t)((uint16_t) xc0));
        spi_transmit((uint8_t)(((uint16_t) xc0) >> 8U));
        spi_transmit((uint8_t)((uint16_t) yc0));
        spi_transmit((uint8_t)(((uint16_t) yc0) >> 8U));
        spi_transmit((uint8_t)((uint16_t) rad));
        spi_transmit((uint8_t)(((uint16_t) rad) >> 8U));
        spi_transmit((uint8_t)(options));
        spi_transmit((uint8_t)(options >> 8U));
        spi_transmit((uint8_t)(hours));
        spi_transmit((uint8_t)(hours >> 8U));
        spi_transmit((uint8_t)(mins));
        spi_transmit((uint8_t)(mins >> 8U));
        spi_transmit((uint8_t)(secs));
        spi_transmit((uint8_t)(secs >> 8U));
        spi_transmit((uint8_t)(msecs));
        spi_transmit((uint8_t)(msecs >> 8U));
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_CLOCK);
        spi_transmit_burst((uint32_t) ((uint16_t) xc0) + (((uint32_t) ((uint16_t) yc0)) << 16U));
        spi_transmit_burst((uint32_t) ((uint16_t) rad) + (((uint32_t) options) << 16U));
        spi_transmit_burst((uint32_t) hours + (((uint32_t) mins) << 16U));
        spi_transmit_burst((uint32_t) secs + (((uint32_t) msecs) << 16U));
    }
}

void EVE_cmd_clock_burst(int16_t xc0, int16_t yc0, int16_t rad, uint16_t options, uint16_t hours,
                            uint16_t mins, uint16_t secs, uint16_t msecs)
{
    spi_transmit_burst(CMD_CLOCK);
    spi_transmit_burst((uint32_t) ((uint16_t) xc0) + (((uint32_t) ((uint16_t) yc0)) << 16U));
    spi_transmit_burst((uint32_t) ((uint16_t) rad) + (((uint32_t) options) << 16U));
    spi_transmit_burst((uint32_t) hours + (((uint32_t) mins) << 16U));
    spi_transmit_burst((uint32_t) secs + (((uint32_t) msecs) << 16U));
}

void EVE_cmd_dial(int16_t xc0, int16_t yc0, int16_t rad, uint16_t options, uint16_t val)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_DIAL);
        spi_transmit((uint8_t)((uint16_t) xc0));
        spi_transmit((uint8_t)(((uint16_t) xc0) >> 8U));
        spi_transmit((uint8_t)((uint16_t) yc0));
        spi_transmit((uint8_t)(((uint16_t) yc0) >> 8U));
        spi_transmit((uint8_t)((uint16_t) rad));
        spi_transmit((uint8_t)(((uint16_t) rad) >> 8U));
        spi_transmit((uint8_t)(options));
        spi_transmit((uint8_t)(options >> 8U));
        spi_transmit((uint8_t)(val));
        spi_transmit((uint8_t)(val >> 8U));
        spi_transmit(0U);
        spi_transmit(0U);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_DIAL);
        spi_transmit_burst((uint32_t) ((uint16_t) xc0) + (((uint32_t) ((uint16_t) yc0)) << 16U));
        spi_transmit_burst((uint32_t) ((uint16_t) rad) + (((uint32_t) options) << 16U));
        spi_transmit_burst(val);
    }
}

void EVE_cmd_dial_burst(int16_t xc0, int16_t yc0, int16_t rad, uint16_t options,
                        uint16_t val)
{
    spi_transmit_burst(CMD_DIAL);
    spi_transmit_burst((uint32_t) ((uint16_t) xc0) + (((uint32_t) ((uint16_t) yc0)) << 16U));
    spi_transmit_burst((uint32_t) ((uint16_t) rad) + (((uint32_t) options) << 16U));
    spi_transmit_burst(val);
}

void EVE_cmd_fgcolor(uint32_t color)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_FGCOLOR);
        spi_transmit((uint8_t)(color));
        spi_transmit((uint8_t)(color >> 8U));
        spi_transmit((uint8_t)(color >> 16U));
        spi_transmit(0U);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_FGCOLOR);
        spi_transmit_burst(color);
    }
}

void EVE_cmd_fgcolor_burst(uint32_t color)
{
    spi_transmit_burst(CMD_FGCOLOR);
    spi_transmit_burst(color);
}

void EVE_cmd_gauge(int16_t xc0, int16_t yc0, int16_t rad, uint16_t options,
                    uint16_t major, uint16_t minor,
                    uint16_t val, uint16_t range)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_GAUGE);
        spi_transmit((uint8_t) ((uint16_t) xc0));
        spi_transmit((uint8_t) (((uint16_t) xc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) yc0));
        spi_transmit((uint8_t) (((uint16_t) yc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) rad));
        spi_transmit((uint8_t) (((uint16_t) rad) >> 8U));
        spi_transmit((uint8_t) (options));
        spi_transmit((uint8_t) (options >> 8U));
        spi_transmit((uint8_t) (major));
        spi_transmit((uint8_t) (major >> 8U));
        spi_transmit((uint8_t) (minor));
        spi_transmit((uint8_t) (minor >> 8U));
        spi_transmit((uint8_t) (val));
        spi_transmit((uint8_t) (val >> 8U));
        spi_transmit((uint8_t) (range));
        spi_transmit((uint8_t) (range >> 8U));
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_GAUGE);
        spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
        spi_transmit_burst(((uint32_t) ((uint16_t) rad)) + (((uint32_t) options) << 16U));
        spi_transmit_burst(((uint32_t) major) + (((uint32_t) minor) << 16U));
        spi_transmit_burst(((uint32_t) val) + (((uint32_t) range) << 16U));
    }
}

void EVE_cmd_gauge_burst(int16_t xc0, int16_t yc0, int16_t rad, uint16_t options,
                            uint16_t major, uint16_t minor,
                            uint16_t val, uint16_t range)
{
    spi_transmit_burst(CMD_GAUGE);
    spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
    spi_transmit_burst(((uint32_t) ((uint16_t) rad)) + (((uint32_t) options) << 16U));
    spi_transmit_burst(((uint32_t) major) + (((uint32_t) minor) << 16U));
    spi_transmit_burst(((uint32_t) val) + (((uint32_t) range) << 16U));
}

/* EVE_cmd_getmatrix() is meant to be called with display-list building. */
/* but it waits for completion as it returns values by writing */
/* to the command-fifo, it can not be used with cmd-burst */
/* gets the properties of the bitmap transform matrix and write the values */
/* to the variables that are supplied by pointers */
void EVE_cmd_getmatrix(int32_t *p_a, int32_t *p_b, int32_t *p_c,
                        int32_t *p_d, int32_t *p_e, int32_t *p_f)
{
    if (0U == cmd_burst)
    {
        uint16_t cmdoffset;
        uint32_t address;

        eve_begin_cmd(CMD_GETMATRIX);
        spi_transmit_32(0UL);
        spi_transmit_32(0UL);
        spi_transmit_32(0UL);
        spi_transmit_32(0UL);
        spi_transmit_32(0UL);
        spi_transmit_32(0UL);
        EVE_cs_clear();
        EVE_execute_cmd();
        cmdoffset = EVE_memRead16(REG_CMD_WRITE);

        if (p_f != NULL)
        {
            address = EVE_RAM_CMD + ((cmdoffset - 4U) & 0xfffU);
            *p_f = (int32_t) EVE_memRead32(address);
        }
        if (p_e != NULL)
        {
            address = EVE_RAM_CMD + ((cmdoffset - 8U) & 0xfffU);
            *p_e = (int32_t) EVE_memRead32(address);
        }
        if (p_d != NULL)
        {
            address = EVE_RAM_CMD + ((cmdoffset - 12U) & 0xfffU);
            *p_d = (int32_t) EVE_memRead32(address);
        }
        if (p_c != NULL)
        {
            address = EVE_RAM_CMD + ((cmdoffset - 16U) & 0xfffU);
            *p_c = (int32_t) EVE_memRead32(address);
        }
        if (p_b != NULL)
        {
            address = EVE_RAM_CMD + ((cmdoffset - 20U) & 0xfffU);
            *p_b = (int32_t) EVE_memRead32(address);
        }
        if (p_a != NULL)
        {
            address = EVE_RAM_CMD + ((cmdoffset - 24U) & 0xfffU);
            *p_a = (int32_t) EVE_memRead32(address);
        }
    }
}

void EVE_cmd_gradcolor(uint32_t color)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_GRADCOLOR);
        spi_transmit((uint8_t)(color));
        spi_transmit((uint8_t)(color >> 8U));
        spi_transmit((uint8_t)(color >> 16U));
        spi_transmit(0U);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_GRADCOLOR);
        spi_transmit_burst(color);
    }
}

void EVE_cmd_gradcolor_burst(uint32_t color)
{
    spi_transmit_burst(CMD_GRADCOLOR);
    spi_transmit_burst(color);
}

void EVE_cmd_gradient(int16_t xc0, int16_t yc0, uint32_t rgb0, int16_t xc1,
                        int16_t yc1, uint32_t rgb1)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_GRADIENT);
        spi_transmit((uint8_t) ((uint16_t) xc0));
        spi_transmit((uint8_t) (((uint16_t) xc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) yc0));
        spi_transmit((uint8_t) (((uint16_t) yc0) >> 8U));
        spi_transmit((uint8_t) (rgb0));
        spi_transmit((uint8_t) (rgb0 >> 8U));
        spi_transmit((uint8_t) (rgb0 >> 16U));
        spi_transmit(0U);
        spi_transmit((uint8_t) ((uint16_t) xc1));
        spi_transmit((uint8_t) (((uint16_t) xc1) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) yc1));
        spi_transmit((uint8_t) (((uint16_t) yc1) >> 8U));
        spi_transmit((uint8_t) (rgb1));
        spi_transmit((uint8_t) (rgb1 >> 8U));
        spi_transmit((uint8_t) (rgb1 >> 16U));
        spi_transmit(0U);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_GRADIENT);
        spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
        spi_transmit_burst(rgb0);
        spi_transmit_burst(((uint32_t) ((uint16_t) xc1)) + (((uint32_t) ((uint16_t) yc1)) << 16U));
        spi_transmit_burst(rgb1);
    }
}

void EVE_cmd_gradient_burst(int16_t xc0, int16_t yc0, uint32_t rgb0, int16_t xc1,
                            int16_t yc1, uint32_t rgb1)
{
    spi_transmit_burst(CMD_GRADIENT);
    spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
    spi_transmit_burst(rgb0);
    spi_transmit_burst(((uint32_t) ((uint16_t) xc1)) + (((uint32_t) ((uint16_t) yc1)) << 16U));
    spi_transmit_burst(rgb1);
}

void EVE_cmd_keys(int16_t xc0, int16_t yc0, int16_t wid, int16_t hgt,
                    int16_t font, uint16_t options, const char *p_text)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_KEYS);
        spi_transmit((uint8_t) ((uint16_t) xc0));
        spi_transmit((uint8_t) (((uint16_t) xc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) yc0));
        spi_transmit((uint8_t) (((uint16_t) yc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) wid));
        spi_transmit((uint8_t) (((uint16_t) wid) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) hgt));
        spi_transmit((uint8_t) (((uint16_t) hgt) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) font));
        spi_transmit((uint8_t) (((uint16_t) font) >> 8U));
        spi_transmit((uint8_t) (options));
        spi_transmit((uint8_t) (options >> 8U));
        private_string_write(p_text);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_KEYS);
        spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
        spi_transmit_burst(((uint32_t) ((uint16_t) wid)) + (((uint32_t) ((uint16_t) hgt)) << 16U));
        spi_transmit_burst(((uint32_t) ((uint16_t) font)) + (((uint32_t) options) << 16U));
        private_string_write(p_text);
    }
}

void EVE_cmd_keys_burst(int16_t xc0, int16_t yc0, int16_t wid, int16_t hgt,
                        int16_t font, uint16_t options, const char *p_text)
{
    spi_transmit_burst(CMD_KEYS);
    spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
    spi_transmit_burst(((uint32_t) ((uint16_t) wid)) + (((uint32_t) ((uint16_t) hgt)) << 16U));
    spi_transmit_burst(((uint32_t) ((uint16_t) font)) + (((uint32_t) options) << 16U));
    private_string_write(p_text);
}

void EVE_cmd_number(int16_t xc0, int16_t yc0, int16_t font, uint16_t options,
                    int32_t number)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_NUMBER);
        spi_transmit((uint8_t) ((uint16_t) xc0));
        spi_transmit((uint8_t) (((uint16_t) xc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) yc0));
        spi_transmit((uint8_t) (((uint16_t) yc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) font));
        spi_transmit((uint8_t) (((uint16_t) font) >> 8U));
        spi_transmit((uint8_t) (options));
        spi_transmit((uint8_t) (options >> 8U));
        spi_transmit_32((uint32_t) number);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_NUMBER);
        spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
        spi_transmit_burst(((uint32_t) ((uint16_t) font)) + (((uint32_t) options) << 16U));
        spi_transmit_burst((uint32_t) number);
    }
}

void EVE_cmd_number_burst(int16_t xc0, int16_t yc0, int16_t font,
                            uint16_t options, int32_t number)
{
    spi_transmit_burst(CMD_NUMBER);
    spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
    spi_transmit_burst(((uint32_t) ((uint16_t) font)) + (((uint32_t) options) << 16U));
    spi_transmit_burst((uint32_t) number);
}

void EVE_cmd_progress(int16_t xc0, int16_t yc0, int16_t wid, int16_t hgt,
                        uint16_t options, uint16_t val, uint16_t range)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_PROGRESS);
        spi_transmit((uint8_t) ((uint16_t) xc0));
        spi_transmit((uint8_t) (((uint16_t) xc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) yc0));
        spi_transmit((uint8_t) (((uint16_t) yc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) wid));
        spi_transmit((uint8_t) (((uint16_t) wid) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) hgt));
        spi_transmit((uint8_t) (((uint16_t) hgt) >> 8U));
        spi_transmit((uint8_t) (options));
        spi_transmit((uint8_t) (options >> 8U));
        spi_transmit((uint8_t) (val));
        spi_transmit((uint8_t) (val >> 8U));
        spi_transmit((uint8_t) (range));
        spi_transmit((uint8_t) (range >> 8U));
        spi_transmit(0U); /* dummy byte for 4-byte alignment */
        spi_transmit(0U); /* dummy byte for 4-byte alignment */
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_PROGRESS);
        spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
        spi_transmit_burst(((uint32_t) ((uint16_t) wid)) + (((uint32_t) ((uint16_t) hgt)) << 16U));
        spi_transmit_burst(((uint32_t) options) + ((uint32_t)val << 16U));
        spi_transmit_burst((uint32_t) range);
    }
}

void EVE_cmd_progress_burst(int16_t xc0, int16_t yc0, int16_t wid, int16_t hgt,
                            uint16_t options, uint16_t val, uint16_t range)
{
    spi_transmit_burst(CMD_PROGRESS);
    spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
    spi_transmit_burst(((uint32_t) ((uint16_t) wid)) + (((uint32_t) ((uint16_t) hgt)) << 16U));
    spi_transmit_burst(((uint32_t) options) + ((uint32_t)val << 16U));
    spi_transmit_burst((uint32_t) range);
}

/* Note: CMD_ROMFONT generates display list commands so it needs to be put in a display list. */
/* A minimum display list to properly execute CMD_ROMFONT would be: */
/* EVE_cmd_dl(CMD_DLSTART); */
/* EVE_cmd_romfont(font, romslot); */
/* ...other bitmap handle commands like more CMD_ROMFONT or CMD_SETFONT2 */
/* EVE_cmd_dl(CMD_SWAP); */
void EVE_cmd_romfont(uint32_t font, uint32_t romslot)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_ROMFONT);
        spi_transmit_32(font);
        spi_transmit_32(romslot);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_ROMFONT);
        spi_transmit_burst(font);
        spi_transmit_burst(romslot);
    }
}

void EVE_cmd_romfont_burst(uint32_t font, uint32_t romslot)
{
    spi_transmit_burst(CMD_ROMFONT);
    spi_transmit_burst(font);
    spi_transmit_burst(romslot);
}

void EVE_cmd_rotate(int32_t angle)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_ROTATE);
        spi_transmit_32((uint32_t) angle);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_ROTATE);
        spi_transmit_burst((uint32_t) angle);
    }
}

void EVE_cmd_rotate_burst(int32_t angle)
{
    spi_transmit_burst(CMD_ROTATE);
    spi_transmit_burst((uint32_t) angle);
}

void EVE_cmd_scale(int32_t scx, int32_t scy)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_SCALE);
        spi_transmit_32((uint32_t) scx);
        spi_transmit_32((uint32_t) scy);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_SCALE);
        spi_transmit_burst((uint32_t) scx);
        spi_transmit_burst((uint32_t) scy);
    }
}

void EVE_cmd_scale_burst(int32_t scx, int32_t scy)
{
    spi_transmit_burst(CMD_SCALE);
    spi_transmit_burst((uint32_t) scx);
    spi_transmit_burst((uint32_t) scy);
}

void EVE_cmd_scrollbar(int16_t xc0, int16_t yc0, int16_t wid, int16_t hgt,
            uint16_t options, uint16_t val, uint16_t size, uint16_t range)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_SCROLLBAR);
        spi_transmit((uint8_t) ((uint16_t) xc0));
        spi_transmit((uint8_t) (((uint16_t) xc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) yc0));
        spi_transmit((uint8_t) (((uint16_t) yc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) wid));
        spi_transmit((uint8_t) (((uint16_t) wid) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) hgt));
        spi_transmit((uint8_t) (((uint16_t) hgt) >> 8U));
        spi_transmit((uint8_t) (options));
        spi_transmit((uint8_t) (options >> 8U));
        spi_transmit((uint8_t) (val));
        spi_transmit((uint8_t) (val >> 8U));
        spi_transmit((uint8_t) (size));
        spi_transmit((uint8_t) (size >> 8U));
        spi_transmit((uint8_t) (range));
        spi_transmit((uint8_t) (range >> 8U));
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_SCROLLBAR);
        spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
        spi_transmit_burst(((uint32_t) ((uint16_t) wid)) + (((uint32_t) ((uint16_t) hgt)) << 16U));
        spi_transmit_burst(((uint32_t) options) + (((uint32_t) val) << 16U));
        spi_transmit_burst(((uint32_t) size) + (((uint32_t) range) << 16U));
    }
}

void EVE_cmd_scrollbar_burst(int16_t xc0, int16_t yc0, int16_t wid, int16_t hgt,
                uint16_t options, uint16_t val, uint16_t size, uint16_t range)
{
    spi_transmit_burst(CMD_SCROLLBAR);
    spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
    spi_transmit_burst(((uint32_t) ((uint16_t) wid)) + (((uint32_t) ((uint16_t) hgt)) << 16U));
    spi_transmit_burst(((uint32_t) options) + (((uint32_t) val) << 16U));
    spi_transmit_burst(((uint32_t) size) + (((uint32_t) range) << 16U));
}

void EVE_cmd_setbase(uint32_t base)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_SETBASE);
        spi_transmit_32(base);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_SETBASE);
        spi_transmit_burst(base);
    }
}

void EVE_cmd_setbase_burst(uint32_t base)
{
    spi_transmit_burst(CMD_SETBASE);
    spi_transmit_burst(base);
}

void EVE_cmd_setbitmap(uint32_t addr, uint16_t fmt, uint16_t width,
                        uint16_t height)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_SETBITMAP);
        spi_transmit_32(addr);
        spi_transmit((uint8_t)(fmt));
        spi_transmit((uint8_t)(fmt >> 8U));
        spi_transmit((uint8_t)(width));
        spi_transmit((uint8_t)(width >> 8U));
        spi_transmit((uint8_t)(height));
        spi_transmit((uint8_t)(height >> 8U));
        spi_transmit(0U);
        spi_transmit(0U);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_SETBITMAP);
        spi_transmit_burst(addr);
        spi_transmit_burst((uint32_t) fmt + (((uint32_t) width) << 16U));
        spi_transmit_burst((uint32_t) height);
    }
}

void EVE_cmd_setbitmap_burst(uint32_t addr, uint16_t fmt, uint16_t width,
                                uint16_t height)
{
    spi_transmit_burst(CMD_SETBITMAP);
    spi_transmit_burst(addr);
    spi_transmit_burst((uint32_t) fmt + (((uint32_t) width) << 16U));
    spi_transmit_burst((uint32_t) height);
}

void EVE_cmd_setfont(uint32_t font, uint32_t ptr)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_SETFONT);
        spi_transmit_32(font);
        spi_transmit_32(ptr);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_SETFONT);
        spi_transmit_burst(font);
        spi_transmit_burst(ptr);
    }
}

void EVE_cmd_setfont_burst(uint32_t font, uint32_t ptr)
{
    spi_transmit_burst(CMD_SETFONT);
    spi_transmit_burst(font);
    spi_transmit_burst(ptr);
}

/* Note: CMD_SETFONT2 generates display list commands so it needs to be put in a display list. */
/* A minimum display list to properly execute CMD_SETFONT2 would be: */
/* EVE_cmd_dl(CMD_DLSTART); */
/* EVE_cmd_setfont2(font, ptr, firstchar); */
/* ...other bitmap handle commands like more CMD_SETFONT2 or CMD_ROMFONT */
/* EVE_cmd_dl(CMD_SWAP); */
void EVE_cmd_setfont2(uint32_t font, uint32_t ptr, uint32_t firstchar)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_SETFONT2);
        spi_transmit_32(font);
        spi_transmit_32(ptr);
        spi_transmit_32(firstchar);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_SETFONT2);
        spi_transmit_burst(font);
        spi_transmit_burst(ptr);
        spi_transmit_burst(firstchar);
    }
}

void EVE_cmd_setfont2_burst(uint32_t font, uint32_t ptr, uint32_t firstchar)
{
    spi_transmit_burst(CMD_SETFONT2);
    spi_transmit_burst(font);
    spi_transmit_burst(ptr);
    spi_transmit_burst(firstchar);
}

void EVE_cmd_setscratch(uint32_t handle)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_SETSCRATCH);
        spi_transmit_32(handle);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_SETSCRATCH);
        spi_transmit_burst(handle);
    }
}

void EVE_cmd_setscratch_burst(uint32_t handle)
{
    spi_transmit_burst(CMD_SETSCRATCH);
    spi_transmit_burst(handle);
}

void EVE_cmd_sketch(int16_t xc0, int16_t yc0, uint16_t wid, uint16_t hgt,
                    uint32_t ptr, uint16_t format)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_SKETCH);
        spi_transmit((uint8_t) ((uint16_t) xc0));
        spi_transmit((uint8_t) (((uint16_t) xc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) yc0));
        spi_transmit((uint8_t) (((uint16_t) yc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) wid));
        spi_transmit((uint8_t) (((uint16_t) wid) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) hgt));
        spi_transmit((uint8_t) (((uint16_t) hgt) >> 8U));
        spi_transmit_32(ptr);
        spi_transmit((uint8_t) (format));
        spi_transmit((uint8_t) (format >> 8U));
        spi_transmit(0U);
        spi_transmit(0U);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_SKETCH);
        spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
        spi_transmit_burst(((uint32_t) ((uint16_t) wid)) + (((uint32_t) ((uint16_t) hgt)) << 16U));
        spi_transmit_burst(ptr);
        spi_transmit_burst((uint32_t) format);
    }
}

void EVE_cmd_sketch_burst(int16_t xc0, int16_t yc0, uint16_t wid, uint16_t hgt,
                            uint32_t ptr, uint16_t format)
{
    spi_transmit_burst(CMD_SKETCH);
    spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
    spi_transmit_burst(((uint32_t) ((uint16_t) wid)) + (((uint32_t) ((uint16_t) hgt)) << 16U));
    spi_transmit_burst(ptr);
    spi_transmit_burst((uint32_t) format);
}

void EVE_cmd_slider(int16_t xc0, int16_t yc0, int16_t wid, int16_t hgt,
                    uint16_t options, uint16_t val, uint16_t range)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_SLIDER);
        spi_transmit((uint8_t) ((uint16_t) xc0));
        spi_transmit((uint8_t) (((uint16_t) xc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) yc0));
        spi_transmit((uint8_t) (((uint16_t) yc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) wid));
        spi_transmit((uint8_t) (((uint16_t) wid) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) hgt));
        spi_transmit((uint8_t) (((uint16_t) hgt) >> 8U));
        spi_transmit((uint8_t) (options));
        spi_transmit((uint8_t) (options >> 8U));
        spi_transmit((uint8_t) (val));
        spi_transmit((uint8_t) (val >> 8U));
        spi_transmit((uint8_t) (range));
        spi_transmit((uint8_t) (range >> 8U));
        spi_transmit(0U); /* dummy byte for 4-byte alignment */
        spi_transmit(0U); /* dummy byte for 4-byte alignment */
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_SLIDER);
        spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
        spi_transmit_burst(((uint32_t) ((uint16_t) wid)) + (((uint32_t) ((uint16_t) hgt)) << 16U));
        spi_transmit_burst(((uint32_t) options) + (((uint32_t) val) << 16U));
        spi_transmit_burst((uint32_t) range);
    }
}

void EVE_cmd_slider_burst(int16_t xc0, int16_t yc0, int16_t wid, int16_t hgt,
                            uint16_t options, uint16_t val, uint16_t range)
{
    spi_transmit_burst(CMD_SLIDER);
    spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
    spi_transmit_burst(((uint32_t) ((uint16_t) wid)) + (((uint32_t) ((uint16_t) hgt)) << 16U));
    spi_transmit_burst(((uint32_t) options) + (((uint32_t) val) << 16U));
    spi_transmit_burst((uint32_t) range);
}

void EVE_cmd_spinner(int16_t xc0, int16_t yc0, uint16_t style, uint16_t scale)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_SPINNER);
        spi_transmit((uint8_t) ((uint16_t) xc0));
        spi_transmit((uint8_t) (((uint16_t) xc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) yc0));
        spi_transmit((uint8_t) (((uint16_t) yc0) >> 8U));
        spi_transmit((uint8_t) (style));
        spi_transmit((uint8_t) (style >> 8U));
        spi_transmit((uint8_t) (scale));
        spi_transmit((uint8_t) (scale >> 8U));
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_SPINNER);
        spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
        spi_transmit_burst(((uint32_t) style) + (((uint32_t) scale) << 16U));
    }
}

void EVE_cmd_spinner_burst(int16_t xc0, int16_t yc0, uint16_t style,
                            uint16_t scale)
{
    spi_transmit_burst(CMD_SPINNER);
    spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
    spi_transmit_burst(((uint32_t) style) + (((uint32_t) scale) << 16U));
}

void EVE_cmd_text(int16_t xc0, int16_t yc0, int16_t font, uint16_t options,
                    const char *p_text)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_TEXT);
        spi_transmit((uint8_t) ((uint16_t) xc0));
        spi_transmit((uint8_t) (((uint16_t) xc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) yc0));
        spi_transmit((uint8_t) (((uint16_t) yc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) font));
        spi_transmit((uint8_t) (((uint16_t) font) >> 8U));
        spi_transmit((uint8_t) (options));
        spi_transmit((uint8_t) (options >> 8U));
        private_string_write(p_text);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_TEXT);
        spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
        spi_transmit_burst(((uint32_t) ((uint16_t) font)) + (((uint32_t) options) << 16U));
        private_string_write(p_text);
    }
}

void EVE_cmd_text_burst(int16_t xc0, int16_t yc0, int16_t font,
                        uint16_t options, const char *p_text)
{
    spi_transmit_burst(CMD_TEXT);
    spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
    spi_transmit_burst(((uint32_t) ((uint16_t) font)) + (((uint32_t) options) << 16U));
    private_string_write(p_text);
}

void EVE_cmd_toggle(int16_t xc0, int16_t yc0, int16_t wid, int16_t font,
                    uint16_t options, uint16_t state, const char *p_text)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_TOGGLE);
        spi_transmit((uint8_t) ((uint16_t) xc0));
        spi_transmit((uint8_t) (((uint16_t) xc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) yc0));
        spi_transmit((uint8_t) (((uint16_t) yc0) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) wid));
        spi_transmit((uint8_t) (((uint16_t) wid) >> 8U));
        spi_transmit((uint8_t) ((uint16_t) font));
        spi_transmit((uint8_t) (((uint16_t) font) >> 8U));
        spi_transmit((uint8_t) (options));
        spi_transmit((uint8_t) (options >> 8U));
        spi_transmit((uint8_t) (state));
        spi_transmit((uint8_t) (state >> 8U));
        private_string_write(p_text);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_TOGGLE);
        spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
        spi_transmit_burst(((uint32_t) ((uint16_t) wid)) + (((uint32_t) ((uint16_t) font)) << 16U));
        spi_transmit_burst(((uint32_t) options) + (((uint32_t) state) << 16U));
        private_string_write(p_text);
    }
}

void EVE_cmd_toggle_burst(int16_t xc0, int16_t yc0, int16_t wid, int16_t font,
                            uint16_t options, uint16_t state, const char *p_text)
{
    spi_transmit_burst(CMD_TOGGLE);
    spi_transmit_burst(((uint32_t) ((uint16_t) xc0)) + (((uint32_t) ((uint16_t) yc0)) << 16U));
    spi_transmit_burst(((uint32_t) ((uint16_t) wid)) + (((uint32_t) ((uint16_t) font)) << 16U));
    spi_transmit_burst(((uint32_t) options) + (((uint32_t) state) << 16U));
    private_string_write(p_text);
}

void EVE_cmd_translate(int32_t tr_x, int32_t tr_y)
{
    if (0U == cmd_burst)
    {
        eve_begin_cmd(CMD_TRANSLATE);
        spi_transmit_32((uint32_t) tr_x);
        spi_transmit_32((uint32_t) tr_y);
        EVE_cs_clear();
    }
    else
    {
        spi_transmit_burst(CMD_TRANSLATE);
        spi_transmit_burst((uint32_t) tr_x);
        spi_transmit_burst((uint32_t) tr_y);
    }
}

void EVE_cmd_translate_burst(int32_t tr_x, int32_t tr_y)
{
    spi_transmit_burst(CMD_TRANSLATE);
    spi_transmit_burst((uint32_t) tr_x);
    spi_transmit_burst((uint32_t) tr_y);
}

void EVE_color_rgb(uint32_t color)
{
    EVE_cmd_dl(DL_COLOR_RGB | (color & 0x00ffffffUL));
}

void EVE_color_rgb_burst(uint32_t color)
{
    spi_transmit_burst(DL_COLOR_RGB | (color & 0x00ffffffUL));
}

void EVE_color_a(uint8_t alpha)
{
    EVE_cmd_dl(DL_COLOR_A | ((uint32_t) alpha));
}

void EVE_color_a_burst(uint8_t alpha)
{
    spi_transmit_burst(DL_COLOR_A | ((uint32_t) alpha));
}


/* ##################################################################
    special purpose functions
##################################################################### */

/* This is meant to be called outside display-list building. */
/* This function displays an interactive calibration screen, calculates the calibration values */
/* and writes the new values to the touch matrix registers of EVE.*/
/* Unlike the built-in cmd_calibrate() of EVE this also works with displays that are cut down from larger ones like
 * EVE2-38A / EVE2-38G. */
/* The dimensions are needed as parameter as EVE_VSIZE for the EVE2-38 is 272 but the visible size is only 116. */
/* So the call would be EVE_calibrate_manual(EVE_HSIZE, 116); for the EVE2-38A and EVE2-38G while for most other
 * displays */
/* using EVE_calibrate_manual(EVE_VSIZE, EVE_VSIZE) would work - but for normal displays the built-in cmd_calibrate
 * would work as expected anyways */
/* This code was taken from the MatrixOrbital EVE2-Library on Github, adapted and modified */
void EVE_calibrate_manual(uint16_t width, uint16_t height)
{
    int32_t display_x[3U];
    int32_t display_y[3U];
    int32_t touch_x[3U];
    int32_t touch_y[3U];
    uint32_t touch_value;
    int32_t tmp;
    int32_t div;
    int32_t trans_matrix[6U];
    uint8_t count = 0U;
    char num[2U];
    uint8_t touch_lock = 1U;

    /* these values determine where your calibration points will be drawn on your display */
    display_x[0U] = (int32_t) (width / 6U);
    display_y[0U] = (int32_t) (height / 6U);

    display_x[1U] = (int32_t) (width - (width / 8U));
    display_y[1U] = (int32_t) (height / 2U);

    display_x[2U] = (int32_t) (width / 2U);
    display_y[2U] = (int32_t) (height - (height / 8U));

    while (count < 3U)
    {
        EVE_cmd_dl(CMD_DLSTART);
        EVE_cmd_dl(DL_CLEAR_COLOR_RGB);
        EVE_cmd_dl(DL_CLEAR | CLR_COL | CLR_STN | CLR_TAG);

        /* draw Calibration Point on screen */
        EVE_cmd_dl(DL_COLOR_RGB | 0x0000ffUL);
        EVE_cmd_dl(POINT_SIZE(15U * 16U));
        EVE_cmd_dl((DL_BEGIN | EVE_POINTS));
        EVE_cmd_dl(VERTEX2F((uint32_t)(display_x[count]) * 16U, (uint32_t)((display_y[count])) * 16U));
        EVE_cmd_dl(DL_END);
        EVE_cmd_dl(DL_COLOR_RGB | 0xffffffUL);
        EVE_cmd_text((int16_t)(width / 2U), 20, 26, EVE_OPT_CENTER, "tap on the dot");
        num[0U] = (char) (count + 0x31U);
        num[1U] = (char) 0U; /* null terminated string of one character */
        EVE_cmd_text((int16_t) display_x[count], (int16_t) display_y[count], 27, EVE_OPT_CENTER, num);

        EVE_cmd_dl(DL_DISPLAY);
        EVE_cmd_dl(CMD_SWAP);
        EVE_execute_cmd();

        for ( ; ; )
        {
            touch_value = EVE_memRead32(REG_TOUCH_DIRECT_XY); /* read for any new touch tag inputs */

            if (touch_lock != 0U)
            {
                if ((touch_value & 0x80000000UL) != 0UL) /* check if we have no touch */
                {
                    touch_lock = 0U;
                }
            }
            else
            {
                if (0UL == (touch_value & 0x80000000UL)) /* check if a touch is detected */
                {
                    touch_x[count] = (int32_t) ((touch_value >> 16U) & 0x03FFU); /* raw Touchscreen Y coordinate */
                    touch_y[count] = (int32_t) (touch_value & 0x03FFU);         /* raw Touchscreen Y coordinate */
                    touch_lock = 1U;
                    count++;
                    break; /* leave for ( ; ; ) */
                }
            }
        }
    }

    div = ((touch_x[0U] - touch_x[2U]) * (touch_y[1U] - touch_y[2U])) - ((touch_x[1U] - touch_x[2U]) * (touch_y[0U] - touch_y[2U]));

    tmp = (((display_x[0U] - display_x[2U]) * (touch_y[1U] - touch_y[2U])) -
           ((display_x[1U] - display_x[2U]) * (touch_y[0U] - touch_y[2U])));
    trans_matrix[0U] = (int32_t) (((int64_t) tmp * 65536) / div);

    tmp = (((touch_x[0U] - touch_x[2U]) * (display_x[1U] - display_x[2U])) -
           ((display_x[0U] - display_x[2U]) * (touch_x[1U] - touch_x[2U])));
    trans_matrix[1U] = (int32_t) (((int64_t) tmp * 65536) / div);

    tmp = ((touch_y[0U] * (((touch_x[2U] * display_x[1U]) - (touch_x[1U] * display_x[2U])))) +
           (touch_y[1U] * (((touch_x[0U] * display_x[2U]) - (touch_x[2U] * display_x[0U])))) +
           (touch_y[2U] * (((touch_x[1U] * display_x[0U]) - (touch_x[0U] * display_x[1U])))));
    trans_matrix[2U] = (int32_t) (((int64_t) tmp * 65536) / div);

    tmp = (((display_y[0U] - display_y[2U]) * (touch_y[1U] - touch_y[2U])) -
           ((display_y[1U] - display_y[2U]) * (touch_y[0U] - touch_y[2U])));
    trans_matrix[3U] = (int32_t) (((int64_t) tmp * 65536) / div);

    tmp = (((touch_x[0U] - touch_x[2U]) * (display_y[1U] - display_y[2U])) -
           ((display_y[0U] - display_y[2U]) * (touch_x[1U] - touch_x[2U])));
    trans_matrix[4U] = (int32_t) (((int64_t) tmp * 65536) / div);

    tmp = ((touch_y[0U] * (((touch_x[2U] * display_y[1U]) - (touch_x[1U] * display_y[2U])))) +
           (touch_y[1U] * (((touch_x[0U] * display_y[2U]) - (touch_x[2U] * display_y[0U])))) +
           (touch_y[2U] * (((touch_x[1U] * display_y[0U]) - (touch_x[0U] * display_y[1U])))));
    trans_matrix[5U] = (int32_t) (((int64_t) tmp * 65536) / div);

    EVE_memWrite32(REG_TOUCH_TRANSFORM_A, (uint32_t) trans_matrix[0U]);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_B, (uint32_t) trans_matrix[1U]);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_C, (uint32_t) trans_matrix[2U]);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_D, (uint32_t) trans_matrix[3U]);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_E, (uint32_t) trans_matrix[4U]);
    EVE_memWrite32(REG_TOUCH_TRANSFORM_F, (uint32_t) trans_matrix[5U]);
}
