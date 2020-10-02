#include "6502_Bus.h"
#include "2C02.h"
#include "6502.h"
#include <stdio.h> // For printf

void cpu_bus_write(Bus6502* bus, uint16_t addr, uint8_t data)
{
	// Check if cartridge wants to handle the write
	bool wrote;
	Cartridge* cart = bus->cartridge;
	cart->CPUWriteCartridge(cart->mapper, addr, data, &wrote);
	if (!wrote)
	{
		// Write to internal RAM
		if (addr >= 0x0000 && addr < 0x2000)
		{
			addr &= 0x07FF;
			bus->memory[addr] = data;
		}
		// Write to PPU registers
		else if (addr >= 0x2000 && addr < 0x4000)
		{
			// Mirror every 8 bytes
			addr = 0x2000 + (addr & 0x07);
			write_ppu(bus->ppu, addr, data);
		}
		// NES APU and I/O registers
		else if (addr >= 0x4000 && addr < 0x4018)
		{
			if (addr == 0x4014)
			{
				bus->cpu->OAMDMA = data;
				bus->cpu->dma_transfer_cycles = ((bus->cpu->total_cycles % 2 == 1) ? 514 : 513);
			}
		}
		// APU and I/O functionality that is normally disabled
		else if (addr >= 0x4018 && addr < 0x4020)
		{

		}
	}
}

uint8_t cpu_bus_read(Bus6502* bus, uint16_t addr)
{
	// Check if cartridge wants to handle the read
	bool read;
	Cartridge* cart = bus->cartridge;
	uint8_t ret = cart->CPUReadCartridge(cart->mapper, addr, &read);
	if (read)
	{
		return ret;
	}
	else
	{
		// Read internal RAM
		if (addr >= 0x0000 && addr < 0x2000)
		{
			addr &= 0x07FF;
			return bus->memory[addr];
		}
		// Read PPU registers
		else if (addr >= 0x2000 && addr < 0x4000)
		{
			// Mirror every 8 bytes
			addr = 0x2000 + (addr & 0x07);
			return read_ppu(bus->ppu, addr);
		}
		// NES APU and I/O registers
		else if (addr >= 0x4000 && addr < 0x4018)
		{
			if (addr == 0x4014)
			{
				printf("OAMDMA\n");
			}
		}
		// APU and I/O functionality that is normally disabled
		else if (addr >= 0x4018 && addr < 0x4020)
		{

		}
	}
	return 0;
}
