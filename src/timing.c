#ifndef __APPLE__
#define _XOPEN_SOURCE   600
#define _POSIX_C_SOURCE 1
#endif
#include "glue.h"
#include "video.h"
#include "cpu/fake6502.h"
#include <stdio.h>
#if ESP_PLATFORM
#	include <Arduino.h>
#	define SDL_GetTicks millis
#	define sleep delay
#	define usleep delayMicroseconds
#else
#	include <SDL.h>
#	include <unistd.h>
#endif

uint32_t frames;
uint32_t sdlTicks_base;
uint32_t last_perf_update;
uint32_t clockticks6502_old;
int64_t cpu_ticks;
int64_t last_perf_cpu_ticks;
char window_title[255];

void
timing_init() {
	frames = 0;
	sdlTicks_base = SDL_GetTicks();
	last_perf_update = 0;
	last_perf_cpu_ticks = 0;
	clockticks6502_old = clockticks6502;
	cpu_ticks = 0;
}

void
timing_update()
{
	frames++;
	cpu_ticks += clockticks6502 - clockticks6502_old;
	clockticks6502_old = clockticks6502;
	uint32_t sdlTicks = SDL_GetTicks() - sdlTicks_base;
	int64_t diff_time = cpu_ticks / MHZ - sdlTicks * 1000LL;
	if (!warp_mode && diff_time > 0) {
		if (diff_time >= 1000000) {
			sleep(diff_time / 1000000);
			diff_time %= 1000000;
		}
		usleep(diff_time);
	}
#if !ESP_PLATFORM
	if (sdlTicks - last_perf_update > 5000) {
		uint32_t perf = (uint32_t) ((cpu_ticks - last_perf_cpu_ticks) / (MHZ * 50000ll));

		if (perf < 100 || warp_mode) {
			sprintf(window_title, WINDOW_TITLE " (%d%%)%s", perf, mouse_grabbed ? MOUSE_GRAB_MSG : "");
		} else {
			sprintf(window_title, WINDOW_TITLE "%s", mouse_grabbed ? MOUSE_GRAB_MSG : "");
		}

		video_update_title(window_title);

		last_perf_cpu_ticks = cpu_ticks;
		last_perf_update = sdlTicks;
	}
#endif
	if (log_speed) {
		static uint32_t oldTicks = 0;
		int32_t frames_behind = (int32_t)(diff_time * 60ll / 1000000ll);
		int load = (int)((1 - frames_behind) * 100);
		printf("Frame %u(ms), Load: %d%%\n", sdlTicks-oldTicks, load > 100 ? 100 : load);
		oldTicks = sdlTicks;
		if (frames_behind < 0) {
			printf("Rendering is behind %d frames.\n", frames_behind);
		} else {
		}
	}
}

