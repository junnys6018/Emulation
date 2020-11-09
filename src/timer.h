#ifndef TIMER_H
#define TIMER_H

#ifdef PLATFORM_WINDOWS
// This struct should be treated as opaque.
// I've exposed its definition so users can create timepoints on the stack
#include <Windows.h>
typedef struct
{
	LARGE_INTEGER tp;
} timepoint;
#endif

#ifdef PLATFORM_LINUX

#include <sys/time.h>
// This struct should be treated as opaque.
// I've exposed its definition so users can create timepoints on the stack
typedef struct timeval timepoint;

#endif

#ifdef __EMSCRIPTEN__
#include <stdint.h>
#include <SDL.h>
typedef uint32_t timepoint;
#endif

#include <stdint.h>

void GetTime(timepoint* tp);
float GetElapsedTimeMicro(timepoint* beg, timepoint* end);
float GetElapsedTimeMilli(timepoint* beg, timepoint* end);

void SleepMicro(uint64_t usec);

#endif // ! TIMER_H
