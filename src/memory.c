// Commander X16 Emulator
// Copyright (c) 2019 Michael Steil
// All rights reserved. License: 2-clause BSD

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "glue.h"
#include "via.h"
#include "memory.h"
#include "video.h"
#include "ymglue.h"
#include "cpu/fake6502.h"
#include "wav_recorder.h"
#include "audio.h"
#include "cartridge.h"

uint8_t ram_bank;
uint8_t rom_bank;

uint8_t *RAM = NULL;
uint8_t *ROM = NULL;
extern uint8_t *CART;

static uint8_t addr_ym = 0;

bool randomizeRAM = false;
bool reportUninitializedAccess = false;
bool *RAM_access_flags;

#define DEVICE_EMULATOR (0x9fb0)

void cpuio_write(uint8_t reg, uint8_t value);

void
memory_init()
{
	// Initialize RAM array
	RAM = calloc(RAM_SIZE, sizeof(uint8_t));
	ROM = calloc(ROM_SIZE, sizeof(uint8_t));
printf("RAM = %p ROM = %p\n", RAM, ROM);
	
	// Randomize all RAM (if option selected)
	if (randomizeRAM) {
		time_t t;
		srand((unsigned)time(&t));
		for (int i = 0; i < RAM_SIZE; i++) {
			RAM[i] = rand();
		}
	}

	// Initialize RAM access flag array (if option selected)
	if (reportUninitializedAccess) {
		RAM_access_flags = (bool*) malloc(RAM_SIZE * sizeof(bool));
		for (int i = 0; i < RAM_SIZE; i++) {
			RAM_access_flags[i] = false;
		}
	}

	memory_reset();
}

void
memory_reset()
{
	// default banks are 0
	memory_set_ram_bank(0);
	memory_set_rom_bank(0);
}

void
memory_report_uninitialized_access(bool value)
{
	reportUninitializedAccess = value;
}

void
memory_randomize_ram(bool value)
{
	randomizeRAM = value;
}

void
memory_initialize_cart(uint8_t *mem)
{
	if(randomizeRAM) {
		for(int i=0; i<0x4000; ++i) {
			mem[i] = rand();
		}
	} else {
		memset(mem, 0, 0x4000);
	}
}

static uint8_t
effective_ram_bank()
{
	return ram_bank;
}

//
// interface for fake6502
//
// if debugOn then reads memory only for debugger; no I/O, no side effects whatsoever

uint8_t
read6502(uint16_t address) {
	// Report access to uninitialized RAM (if option selected)
	if (reportUninitializedAccess) {
		uint8_t pc_bank;
		
		if (pc < 0xa000) {
			pc_bank = 0;
		} else if (pc < 0xc000) {
			pc_bank = memory_get_ram_bank();
		} else {
			pc_bank = memory_get_rom_bank();
		}

		if (address < 0x9f00) {
			if (RAM_access_flags[address] == false) {
				printf("Warning: %02X:%04X accessed uninitialized RAM address 00:%04X\n", pc_bank, pc, address);
			}
		} else if (address >= 0xa000 && address < 0xc000) {
			if (effective_ram_bank() < num_ram_banks && RAM_access_flags[0xa000 + (effective_ram_bank() << 13) + address - 0xa000] == false){
				printf("Warning: %02X:%04X accessed uninitialized RAM address %02X:%04X\n", pc_bank, pc, memory_get_ram_bank(), address);
			}
		}
	}

	return real_read6502(address, false, 0);
}

uint8_t
real_read6502(uint16_t address, bool debugOn, uint8_t bank)
{
	if (address < 0x9f00) { // RAM
		return RAM[address];
	} else if (address < 0xa000) { // I/O
		if (!debugOn && address >= 0x9fa0) {
			// slow IO6-8 range
			clockticks6502 += 3;
		}
		if (address >= 0x9f00 && address < 0x9f10) {
			return via1_read(address & 0xf, debugOn);
		} else if (has_via2 && (address >= 0x9f10 && address < 0x9f20)) {
			return via2_read(address & 0xf, debugOn);
		} else if (address >= 0x9f20 && address < 0x9f40) {
			return video_read(address & 0x1f, debugOn);
		} else if (address >= 0x9f40 && address < 0x9f60) {
			// slow IO3 range
			if (!debugOn) {
				clockticks6502 += 3;
			}
			if (address == 0x9f41) {
				audio_render();
				return YM_read_status();
			}
			return 0x9f; // open bus read
		} else if (address >= 0x9fb0 && address < 0x9fc0) {
			// emulator state
			return emu_read(address & 0xf, debugOn);
		} else {
			// future expansion
			return 0x9f; // open bus read
		}
	} else if (address < 0xc000) { // banked RAM
		int ramBank = debugOn ? bank : effective_ram_bank();
		if (ramBank < num_ram_banks) {
			return RAM[0xa000 + (ramBank << 13) + address - 0xa000];
		} else {
			return (address >> 8) & 0xff; // open bus read
		}


	} else { // banked ROM
		int romBank = debugOn ? bank : rom_bank;
		if (romBank < 32) {
			return ROM[(romBank << 14) + address - 0xc000];
		} else {
			if (!CART) {
				return (address >> 8) & 0xff; // open bus read
			}
			return cartridge_read(address, romBank);
		}
	}
}

void
write6502(uint16_t address, uint8_t value)
{
	// Update RAM access flag
	if (reportUninitializedAccess) {
		if (address < 0xa000) {
			RAM_access_flags[address] = true;
		} else if (address < 0xc000) {
			if (effective_ram_bank() < num_ram_banks)
				RAM_access_flags[0xa000 + (effective_ram_bank() << 13) + address - 0xa000] = true;
		}
	}
	// Write to CPU I/O ports
	if (address < 2) { 
		cpuio_write(address, value);
	}
	// Write to memory
	if (address < 0x9f00) { // RAM
		RAM[address] = value;
	} else if (address < 0xa000) { // I/O
		if (address >= 0x9fa0) {
			// slow IO6-8 range
			clockticks6502 += 3;
		}
		if (address >= 0x9f00 && address < 0x9f10) {
			via1_write(address & 0xf, value);
		} else if (has_via2 && (address >= 0x9f10 && address < 0x9f20)) {
			via2_write(address & 0xf, value);
		} else if (address >= 0x9f20 && address < 0x9f40) {
			video_write(address & 0x1f, value);
		} else if (address >= 0x9f40 && address < 0x9f60) {
			// slow IO3 range
			clockticks6502 += 3;
			if (address == 0x9f40) {        // YM address
				addr_ym = value;
			} else if (address == 0x9f41) { // YM data
				audio_render();
				YM_write_reg(addr_ym, value);
			}
			// TODO:
			//   $9F42 & $9F43: SAA1099P
		} else if (address >= 0x9fb0 && address < 0x9fc0) {
			// emulator state
			emu_write(address & 0xf, value);
		} else {
			// future expansion
		}
	} else if (address < 0xc000) { // banked RAM
		if (effective_ram_bank() < num_ram_banks)
			RAM[0xa000 + (effective_ram_bank() << 13) + address - 0xa000] = value;
	} else { // ROM
		if (rom_bank >= 32) { // Cartridge ROM/RAM
			cartridge_write(address, rom_bank, value);
		}
		// ignore if base ROM (banks 0-31)
	}
}

void
vp6502()
{
	memory_set_rom_bank(0);
}

//
// saves the memory content into a file
//

void
memory_save(SDL_RWops *f, bool dump_ram, bool dump_bank)
{
	if (dump_ram) {
		SDL_RWwrite(f, &RAM[0], sizeof(uint8_t), 0xa000);
	}
	if (dump_bank) {
		SDL_RWwrite(f, &RAM[0xa000], sizeof(uint8_t), (num_ram_banks * 8192));
	}
}


///
///
///

void
memory_set_ram_bank(uint8_t bank)
{
	ram_bank = bank & (NUM_MAX_RAM_BANKS - 1);
}

uint8_t
memory_get_ram_bank()
{
	return ram_bank;
}

void
memory_set_rom_bank(uint8_t bank)
{
	rom_bank = bank;
}

uint8_t
memory_get_rom_bank()
{
	return rom_bank;
}

void
cpuio_write(uint8_t reg, uint8_t value)
{
	switch (reg) {
		case 0:
			memory_set_ram_bank(value);
			break;
		case 1:
			memory_set_rom_bank(value);
			break;
	}
}

// Control the GIF recorder
void
emu_recorder_set(gif_recorder_command_t command)
{
	// turning off while recording is enabled
	if (command == RECORD_GIF_PAUSE && record_gif != RECORD_GIF_DISABLED) {
		record_gif = RECORD_GIF_PAUSED; // need to save
	}
	// turning on continuous recording
	if (command == RECORD_GIF_RESUME && record_gif != RECORD_GIF_DISABLED) {
		record_gif = RECORD_GIF_ACTIVE;		// activate recording
	}
	// capture one frame
	if (command == RECORD_GIF_SNAP && record_gif != RECORD_GIF_DISABLED) {
		record_gif = RECORD_GIF_SINGLE;		// single-shot
	}
}

//
// read/write emulator state (feature flags)
//
// 0: debugger_enabled
// 1: log_video
// 2: log_keyboard
// 3: echo_mode
// 4: save_on_exit
// 5: record_gif
// 6: record_wav
// POKE $9FB3,1:PRINT"ECHO MODE IS ON":POKE $9FB3,0
void
emu_write(uint8_t reg, uint8_t value)
{
	bool v = value != 0;
	switch (reg) {
		case 0: debugger_enabled = v; break;
		case 1: log_video = v; break;
		case 2: log_keyboard = v; break;
		case 3: echo_mode = value; break;
		case 4: save_on_exit = v; break;
		case 5: emu_recorder_set((gif_recorder_command_t) value); break;
		case 6: wav_recorder_set((wav_recorder_command_t) value); break;
		case 7: disable_emu_cmd_keys = v; break;
		default: printf("WARN: Invalid register %x\n", DEVICE_EMULATOR + reg);
	}
}

uint8_t
emu_read(uint8_t reg, bool debugOn)
{
	if (reg == 0) {
		return debugger_enabled ? 1 : 0;
	} else if (reg == 1) {
		return log_video ? 1 : 0;
	} else if (reg == 2) {
		return log_keyboard ? 1 : 0;
	} else if (reg == 3) {
		return echo_mode;
	} else if (reg == 4) {
		return save_on_exit ? 1 : 0;
	} else if (reg == 5) {
		return record_gif;
	} else if (reg == 6) {
		return wav_recorder_get_state();
	} else if (reg == 7) {
		return disable_emu_cmd_keys ? 1 : 0;

	} else if (reg == 8) {
		return (clockticks6502 >> 0) & 0xff;
	} else if (reg == 9) {
		return (clockticks6502 >> 8) & 0xff;
	} else if (reg == 10) {
		return (clockticks6502 >> 16) & 0xff;
	} else if (reg == 11) {
		return (clockticks6502 >> 24) & 0xff;

	} else if (reg == 13) {
		return keymap;
	} else if (reg == 14) {
		return '1'; // emulator detection
	} else if (reg == 15) {
		return '6'; // emulator detection
	}
	if (!debugOn) printf("WARN: Invalid register %x\n", DEVICE_EMULATOR + reg);
	return -1;
}
