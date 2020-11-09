#include "Frontend/Renderer.h"
#include "Frontend/Gui.h"
#include "Frontend/Controller.h"
#include "Frontend/Audio.h"

#include "Backend/nes.h"

#include "test_6502.h"
#include "test_2C02.h"
#include "Benchmarks.h"
#include "timer.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

#include <SDL.h>

typedef struct
{
	Nes nes;
	Controller controller;
	timepoint beg, end;
} FrameUserdata;

void one_frame(void* data)
{
	static const int window = 10;
	static float total_time = 0.0f;
	static int curr_frame = 0;

	FrameUserdata* userdata = (FrameUserdata*)data;

	GetTime(&userdata->beg);
	poll_keys(&userdata->nes.pad);

	Nes* nes = &userdata->nes;

	if (userdata->controller.mode == MODE_PLAY)
	{
		clock_nes_frame(nes);
		if (userdata->nes.apu.audio_pos != 0)
		{
			WriteSamples(nes->apu.audio_buffer, nes->apu.audio_pos);
			nes->apu.audio_pos = 0;
		}
	}

	RendererDraw();

	SDL_Event event;
	while (SDL_PollEvent(&event) != 0)
	{
		GuiDispatchEvent(&event);
		if (userdata->controller.mode == MODE_STEP_THROUGH && event.type == SDL_KEYDOWN)
		{
			switch (event.key.keysym.sym)
			{
			case SDLK_SPACE:
				clock_nes_instruction(nes);
				break;
			case SDLK_f:
				clock_nes_frame(nes);
				break;
			case SDLK_p:
				clock_nes_cycle(nes);
				break;
			}
		}
		else if (event.type == SDL_QUIT)
		{
			NESDestroy(nes);
			RendererShutdown();
			ShutdownSDLAudio();
			SDL_Quit();
			free(userdata);

			exit(EXIT_SUCCESS);
		}
	}

	GetTime(&userdata->end);
	total_time += GetElapsedTimeMilli(&userdata->beg, &userdata->end);
	curr_frame++;
	if (curr_frame == window)
	{
		userdata->controller.ms_per_frame = total_time / window;

		total_time = 0.0f;
		curr_frame = 0;
	}
}

int main(int argc, char** argv)
{
	// Initialize all the things
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) != 0)
	{
		printf("[ERROR] Could not initialize SDL");
		exit(EXIT_FAILURE);
	}
	FrameUserdata* userdata = malloc(sizeof(FrameUserdata));
	assert(userdata);
	userdata->controller.mode = MODE_NOT_RUNNING;
	RendererInit(&userdata->controller);
	AudioPrecompute();
	InitSDLAudio();
	NESInit(&userdata->nes, NULL);
	RendererBindNES(&userdata->nes);
	
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop_arg(one_frame, userdata, 60, 0);
#else
	if (argc == 2 && strcmp(argv[1], "--test") == 0)
	{
		RunAll6502Tests();
		RunAll2C02Tests();
		RendererBindNES(&userdata->nes);
	}
	else if (argc == 2 && strcmp(argv[1], "--benchmark") == 0)
	{
		RunAllBenchmarks();
		RendererBindNES(&userdata->nes);
	}
	else if (argc == 2)
	{
		NESDestroy(&userdata->nes);
		if (NESInit(&userdata->nes, argv[1]) == 0)
		{
			userdata->controller.mode = MODE_PLAY;
		}
		else
		{
			printf("[ERROR] Failed to load %s as an ines rom\n", argv[1]);
			NESInit(&userdata->nes, NULL);
		}
	}
	while (true)
	{
		one_frame(userdata);

		float elapsed = GetElapsedTimeMicro(&userdata->beg, &userdata->end);
		if (elapsed < 16666) // 60 FPS
		{
			SleepMicro((uint64_t)(16666 - elapsed));
		}
	}
#endif
}
