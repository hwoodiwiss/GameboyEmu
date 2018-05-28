#include "MMU.h"

MMU::MMU()
{
	memAllocator = 0xC000;
	LoadGraphic();
}

MMU::~MMU()
{
	delete memory;
}

void MMU::LoadBootstrap()
{
	memContext = memAllocator;
	memAllocator = 0x0;

	for (WORD i = 0; i < 256; i++)
	{
		WriteByte(BootstrapRom[i]);
	}

	memAllocator = memContext;

}

void MMU::LoadGraphic()
{
	memContext = memAllocator;
	memAllocator = 0x104;

	for (BYTE i = 0; i < sizeof(scrollingGraphic); i++)
	{
		WriteByte(scrollingGraphic[i]);
	}

	memAllocator = memContext;
}

WORD MMU::WriteByte(BYTE val)
{
	memory[memAllocator] = val;
	return memAllocator++;
}

WORD MMU::WriteWord(WORD val)
{
	//Stored least significant first to emulate little-endianness
	memory[memAllocator] = (BYTE)(val);

	//Right shifted 8 bits to ignore the least significant byte
	//EG. 2000 = 00001011 1110000
	// 00001011 11100000 >> 8 = 11100000 00000000
	//Cast to byte and we store 11100000
	memory[memAllocator + 1] = (BYTE)(val >> 8);
	memAllocator += 2;
	return memAllocator - 2;
}

void MMU::WriteByte(WORD addr, BYTE val)
{
	memory[addr] = val;
}

void MMU::WriteWord(WORD addr, WORD val)
{
	//Stored least significant first to emulate little-endianness
	memory[addr] = (BYTE)(val);

	//Right shifted 8 bits to ignore the least significant byte
	//EG. 2000 = 00001011 1110000
	// 00001011 11100000 >> 8 = 11100000 00000000
	//Cast to byte and we store 11100000
	memory[addr + 1] = (BYTE)(val >> 8);
}

BYTE MMU::ReadByte(WORD addr)
{
	return memory[addr];
}

WORD MMU::ReadWord(WORD addr)
{
	//Intel x86 is little endian
	//So the high part (memory[addr + 1] gets left shifted 8 bit which results with 8 trailing 0's followed by the value held in the mem location
	//EG. 2000 = 00001011 11100000
	//Low: 0000 1011 High: 11100000
	//High << 8: 00000000 11100000
	//When Or'd with the low part, the lowpart value will now slot into the trailing zeros
	//The implied end outcome for the OR operation is: 00001011 00000000 OR 00000000 11100000
	//Which gives: 00001011 11100000
	//Moved to preprocessor in Shared.h
	return BytesToWord(memory[addr], memory[addr + 1]);
}
