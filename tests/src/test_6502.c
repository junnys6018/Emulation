#include "test_6502.h"
#include "nes.h"
#include "timer.h"
#include "test_util.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>


int Run_6502_Functional_Test()
{
	Nes nes;
	InitNES(&nes, "roms/6502_functional_test.bin", NULL);

	EmulateUntilHalt(&nes);

	// Check for success or failure
	uint8_t success_opcode_pattern[] = { 0x4C, 0xBA, 0xEA, 0xBA, 0xEA };
	uint8_t opcode_size[] = { 3, 1, 1, 1, 1 };
	bool passed = true;
	uint16_t pc = nes.cpu.PC;
	for (int i = 0; i < 5; i++)
	{
		uint8_t opcode = cpu_bus_read(&nes.cpu_bus, pc);
		passed = passed && (opcode == success_opcode_pattern[i]);
		pc += opcode_size[i];
	}
	if (passed)
		printf("Passed Functional Test\n");
	else
		printf("Failed Functional Test\n");

	
	NESDestroy(&nes);

	return passed ? 0 : 1;
}

int Run_6502_Interrupt_Test()
{
	Nes nes;
	InitNES(&nes, "roms/6502_interrupt_test.bin", NULL);

	const uint16_t feedback_register_addr = 0xBFFC; // Location of register used to feed interrupts into the cpu
	uint8_t old_nmi = 0; // Used to detect a level change
	uint16_t old_PC = 0x400; // Used to detect if cpu has halted

	bool passed = true;
	while (true)
	{
		while (clock_6502(&nes.cpu) != 0);
		uint8_t I_src = cpu_bus_read(&nes.cpu_bus, feedback_register_addr);
		if (I_src & 0x02 && old_nmi == 0) // NMI - Detected on rising edge
		{
			NMI(&nes.cpu);
		}
		if (I_src & 0x01) // IRQ - level detected
		{
			IRQ_Set(&nes.cpu, 7);
		}
		else
		{
			IRQ_Clear(&nes.cpu, 7);
		}

		old_nmi = I_src & 0x02; // Mask out NMI bit

		// Check for success or failure
		if (old_PC == nes.cpu.PC) // Execution paused in infinite loop
		{
			uint8_t success_opcode_pattern[] = {0x4C, 0xBA, 0xEA, 0xBA, 0xEA};
			uint8_t opcode_size[] = {3, 1, 1, 1, 1};
			for (int i = 0; i < 5; i++)
			{
				uint8_t opcode = cpu_bus_read(&nes.cpu_bus, old_PC);
				passed = passed && (opcode == success_opcode_pattern[i]);
				old_PC += opcode_size[i];
			}

			if (passed)
				printf("Passed Interrupt Test\n");
			else
				printf("Failed Interrupt Test\n");

			break;
		}

		old_PC = nes.cpu.PC;
	}

	NESDestroy(&nes);
	return passed ? 0 : 1;
}


void RunAll6502Tests()
{
	const int num_tests = 5;
	int num_failed = 0;
	num_failed += Run_6502_Interrupt_Test();
	num_failed += Run_6502_Functional_Test();

	num_failed += TestBlarggRom("roms/blargg_tests/branch_timing_tests/1.Branch_Basics.nes", 0xF8);
	num_failed += TestBlarggRom("roms/blargg_tests/branch_timing_tests/2.Backward_Branch.nes", 0xF8);
	num_failed += TestBlarggRom("roms/blargg_tests/branch_timing_tests/3.Forward_Branch.nes", 0xF8);

	printf("[6502 TESTS] Passed: %i; Failed %i\n", num_tests - num_failed, num_failed);
}
