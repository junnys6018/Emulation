#ifndef _6502_H
#define _6502_H
#include "bus.h"

typedef struct
{
	uint8_t A; // Accumilator register 
	uint8_t X; // X and Y index registers
	uint8_t Y;
	uint16_t PC; // Program counter
	uint8_t SP; // Stack pointer

	union
	{
		struct
		{
			uint8_t N : 1;		// Negative
			uint8_t V : 1;		// Overflow
			uint8_t unused : 2;
			uint8_t D : 1;		// Decimal
			uint8_t I : 1;		// Interrupt disable
			uint8_t Z : 1;		// Zero
			uint8_t C : 1;		// Carry
		} flags;
		uint8_t reg;
	} status;

	Bus* bus;

	// Helper members
	uint8_t operand; // Data fetched for the operand of an instruction
	uint16_t addr; // Address of the data fetched, only set for zero page, absolute addressing and indirect X, Y 

	/* Data fetched from an absolute indirect addressing mode operation, unlike other addressing modes
	 * 2 bytes are fetched, note only the JMP instruction uses absolute indirect addressing
	 */
	uint16_t indirect_fetch;
	
	int32_t remaining_cycles; // Remaining cycles in the current instruction
	int32_t total_cycles;

} State6502;

void clock(State6502* cpu);

#endif

