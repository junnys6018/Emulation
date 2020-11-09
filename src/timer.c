#include "timer.h"

#ifdef PLATFORM_WINDOWS

static LARGE_INTEGER freq = { 0 };

void GetTime(timepoint* tp)
{
	QueryPerformanceCounter(&tp->tp);
}

float GetElapsedTimeMicro(timepoint* beg, timepoint* end)
{
	if (freq.QuadPart == 0)
	{
		QueryPerformanceFrequency(&freq);
	}

	return (float)(end->tp.QuadPart - beg->tp.QuadPart) / freq.QuadPart * 1000000;
}

float GetElapsedTimeMilli(timepoint* beg, timepoint* end)
{
	if (freq.QuadPart == 0)
	{
		QueryPerformanceFrequency(&freq);
	}

	return (float)(end->tp.QuadPart - beg->tp.QuadPart) / freq.QuadPart * 1000;
}

void SleepMicro(uint64_t usec)
{
	HANDLE timer;
	LARGE_INTEGER ft;

	ft.QuadPart = -(10 * (LONGLONG)usec); // Convert to 100 nanosecond interval, negative value indicates relative time

	timer = CreateWaitableTimer(NULL, TRUE, NULL);
	SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
	WaitForSingleObject(timer, INFINITE);
	CloseHandle(timer);
}

#endif

#ifdef PLATFORM_LINUX

#include <stddef.h>

void GetTime(timepoint* tp)
{
	gettimeofday(tp, NULL);
}

float GetElapsedTimeMicro(timepoint* beg, timepoint* end)
{
	long beg_time = 1000000 * beg->tv_sec + beg->tv_usec;
	long end_time = 1000000 * end->tv_sec + end->tv_usec;

	return end_time - beg_time;
}
float GetElapsedTimeMilli(timepoint* beg, timepoint* end)
{
	return GetElapsedTimeMicro(beg,end) / 1000.0f;
}

#include <unistd.h>
void SleepMicro(uint64_t usec)
{
	usleep(usec);
}
#endif

#ifdef __EMSCRIPTEN__
void GetTime(timepoint* tp)
{
	*tp = SDL_GetTicks();
}

float GetElapsedTimeMicro(timepoint* beg, timepoint* end)
{
	return 1000.0f * GetElapsedTimeMilli(beg, end);
}

float GetElapsedTimeMilli(timepoint* beg, timepoint* end)
{
	return *end - *beg;
}

void SleepMicro(uint64_t usec)
{
	SDL_Delay(usec / 1000);
}
#endif