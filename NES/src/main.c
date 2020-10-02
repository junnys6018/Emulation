#include "Frontend/Renderer.h"
#include "Frontend/Gui.h"

#include "event_filter_function.h"

#include "Backend/Mappers/Mapper_000.h"
#include "Backend/nes.h"

#include "test_6502.h"
#include "test_2C02.h"
#include "Benchmarks.h"

#include "timer.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <SDL.h>

int main(int argc, char** argv)
{
	RendererInit();

	//RunAll6502Tests();
	//RunAll2C02Tests();
	//RunAllBenchmarks();

	Nes nes;
	//NESInit(&nes, "roms/DonkeyKong.nes");
	//NESInit(&nes, "tests/roms/oam_stress.nes");
	NESInit(&nes, "tests/roms/nmi_sync.nes");
	//NESInit(&nes, "tests/roms/blargg_vbl_nmi_timing/sprite_ram.nes");

	RendererBindNES(&nes);

	// TODO: 
	uint8_t* chr = ((Mapper000*)(nes.cart.mapper))->CHR;
	RendererSetPatternTable(chr, 0);
	RendererSetPatternTable(chr + 0x1000, 1);
	SDL_Event event;
	while (true)
	{
		//timepoint beg, end;
		//GetTime(&beg);
		//clock_nes_frame(&nes);
		//GetTime(&end);
		//printf("time: %.3fms\n", GetElapsedTimeMilli(&beg, &end));

		RendererDraw();
		while (SDL_PollEvent(&event) != 0)
		{
			GuiDispatchEvent(&event);
			if (event.type == SDL_KEYDOWN)
			{
				switch (event.key.keysym.sym)
				{
				case SDLK_SPACE:
					clock_nes_instruction(&nes);
					break;
				case SDLK_f:
				{
					clock_nes_frame(&nes);
					break;
				}
				case SDLK_p:
					clock_nes_cycle(&nes);
					break;
				}
			}
			if (event.type == SDL_QUIT)
			{
				exit(EXIT_SUCCESS);
			}
		}
	}

	NESDestroy(&nes);
	RendererShutdown();

	exit(EXIT_SUCCESS);
}
