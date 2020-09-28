#include "test.h"

#include "Frontend/Renderer.h"
#include "Backend/6502.h"
#include "event_filter_function.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#include <SDL.h>

Uint32 on_render_callback(Uint32 interval, void* param)
{
	SDL_Event event;
	SDL_UserEvent userevent;

	userevent.type = SDL_USEREVENT;
	userevent.code = 0;
	userevent.data1 = NULL;
	userevent.data2 = NULL;

	event.type = SDL_USEREVENT;
	event.user = userevent;

	SDL_PushEvent(&event);

	return interval;
}

void Run_6502_Functional_Test()
{
	Cartridge cart;
	load_cartridge_from_file(&cart, "tests/roms/6502_functional_test.bin");

	// TODO: put this into a function
	Bus6502 cpu_bus;
	Bus2C02 ppu_bus;
	State6502 cpu;
	State2C02 ppu;

	cpu_bus.cartridge = &cart;
	cpu_bus.ppu = &ppu;

	ppu_bus.cartridge = &cart;

	cpu.bus = &cpu_bus;

	ppu.bus = &ppu_bus;
	ppu.cpu = &cpu;

	power_on_6502(&cpu);
	reset_6502(&cpu);

	power_on_2C02(&ppu);
	reset_2C02(&ppu);

	RendererSetPaletteData(ppu_bus.palette);

	// Update screen every 16ms
	SDL_TimerID tid = SDL_AddTimer(16, on_render_callback, NULL);

	// Filter away events that are not used
	EventTypeList list = { .size = 2,.event_types = {SDL_USEREVENT, SDL_QUIT} };
	SDL_SetEventFilter(event_whitelist, &list);

	// Used to detect if cpu has halted
	uint16_t old_PC = 0x400;

	// Execution speed
	const int instructions_per_frame = 200000;
	int instructions_done = 0;

	SDL_Event event;
	RendererDraw(&cpu, &ppu);
	while (true)
	{
		if (instructions_done < instructions_per_frame)
		{
			// Perform 5000 instructions at once
			for (int i = 0; i < 5000; i++)
			{
				instructions_done++;
				while(clock_6502(&cpu) != 0);
			}

			// Check for success or failure
			if (old_PC == cpu.PC) // Execution paused in infinite loop
			{
				uint8_t success_opcode_pattern[] = { 0x4C, 0xBA, 0xEA, 0xBA, 0xEA };
				uint8_t opcode_size[] = { 3, 1, 1, 1, 1 };
				bool passed = true;
				for (int i = 0; i < 5; i++)
				{
					uint8_t opcode = cpu_bus_read(cpu.bus, old_PC);
					passed = passed && (opcode == success_opcode_pattern[i]);
					old_PC += opcode_size[i];
				}

				if (passed)
					printf("Passed Functional Test\n");
				else
					printf("Failed Functional Test\n");

				RendererDraw(&cpu, &ppu);
				break;
			}
			old_PC = cpu.PC;
		}

		// Poll Events
		while (SDL_PollEvent(&event) != 0)
		{
			if (event.type == SDL_QUIT)
			{
				exit(EXIT_SUCCESS);
			}
			else if (event.type == SDL_USEREVENT && event.user.code == 0)
			{
				RendererDraw(&cpu, &ppu);
				//printf("done: %i\n", instructions_done);
				instructions_done = 0;
			}
		}

	}

	free_cartridge(&cart);

	SDL_RemoveTimer(tid);
	SDL_SetEventFilter(reset_filter_event, NULL);
}

void Run_6502_Interrupt_Test()
{
	Cartridge cart;
	load_cartridge_from_file(&cart, "tests/roms/6502_interrupt_test.bin");

	// TODO: put this into a function
	Bus6502 cpu_bus;
	Bus2C02 ppu_bus;
	State6502 cpu;
	State2C02 ppu;

	cpu_bus.cartridge = &cart;
	cpu_bus.ppu = &ppu;

	ppu_bus.cartridge = &cart;

	cpu.bus = &cpu_bus;

	ppu.bus = &ppu_bus;
	ppu.cpu = &cpu;

	power_on_6502(&cpu);
	reset_6502(&cpu);

	power_on_2C02(&ppu);
	reset_2C02(&ppu);

	RendererSetPaletteData(ppu_bus.palette);

	// Update screen every 16ms
	SDL_TimerID tid = SDL_AddTimer(16, on_render_callback, NULL);

	// Filter away events that are not used
	EventTypeList list = { .size = 2,.event_types = {SDL_USEREVENT, SDL_QUIT} };
	SDL_SetEventFilter(event_whitelist, &list);


	const uint16_t feedback_register_addr = 0xBFFC; // Location of register used to feed interrupts into the cpu
	uint8_t old_nmi = 0; // Used to detect a level change
	uint16_t old_PC = 0x400; // Used to detect if cpu has halted
	const int instructions_per_frame = 10; // Execution speed
	int instructions_done = 0;

	SDL_Event event;
	RendererDraw(&cpu, &ppu);
	while (true)
	{
		if (instructions_done < instructions_per_frame)
		{
			instructions_done++;

			while (clock_6502(&cpu) != 0);
			uint8_t I_src = cpu_bus_read(cpu.bus, feedback_register_addr);
			if (I_src & 0x02 && old_nmi == 0) // NMI - Detected on rising edge
			{
				NMI(&cpu);
			}
			if (I_src & 0x01) // IRQ - level detected
			{
				IRQ(&cpu);
			}

			old_nmi = I_src & 0x02; // Mask out NMI bit

			// Check for success or failure
			if (old_PC == cpu.PC) // Execution paused in infinite loop
			{
				uint8_t success_opcode_pattern[] = { 0x4C, 0xBA, 0xEA, 0xBA, 0xEA };
				uint8_t opcode_size[] = { 3, 1, 1, 1, 1 };
				bool passed = true;
				for (int i = 0; i < 5; i++)
				{
					uint8_t opcode = cpu_bus_read(cpu.bus, old_PC);
					passed = passed && (opcode == success_opcode_pattern[i]);
					old_PC += opcode_size[i];
				}

				if (passed)
					printf("Passed Interrupt Test\n");
				else
					printf("Failed Interrupt Test\n");

				RendererDraw(&cpu, &ppu);
				break;
			}

			old_PC = cpu.PC;
		}


		// Poll Events
		while (SDL_PollEvent(&event) != 0)
		{
			if (event.type == SDL_QUIT)
			{
				exit(EXIT_SUCCESS);
			}
			else if (event.type == SDL_USEREVENT && event.user.code == 0)
			{
				RendererDraw(&cpu, &ppu);
				instructions_done = 0;
			}
		}

	}

	free_cartridge(&cart);

	SDL_RemoveTimer(tid);
	SDL_SetEventFilter(reset_filter_event, NULL);
}


void Run_All_Tests()
{
	Run_6502_Interrupt_Test();
	Run_6502_Functional_Test();
}
