#include "MMU.h"
#include <iostream>
#include <fstream>

MMU::MMU()
{
	memory = std::unique_ptr<BYTE[]>(new BYTE[0xFFFF]);
	bootromEnabled = false;
	//LoadGraphic();
}

void MMU::LoadBootstrap()
{

	for (WORD i = 0; i < 0xFF; i++)
	{
		WriteByte(i, BootstrapRom[i]);
	}

}


bool MMU::LoadROM(const char* fileName)
{
	std::ifstream romFile;
	romFile.open(fileName, std::ios::binary);
	if (romFile.fail())
	{
		printf("Error opening file!\n");
		return false;
	}

	WORD ptr = 0x0100;
	for (ptr; ptr < 0x14F; ptr++)
	{
		char nxtByte;
		romFile.read(&nxtByte, 1);
		memory[ptr] = nxtByte;
	}

	for (ptr; ptr < 0x3FFF; ptr++)
	{
		char nxtByte;
		romFile.read(&nxtByte, 1);
		memory[ptr] = nxtByte;
	}

	return true;
}

bool MMU::LoadROM(char* fileName)
{
	std::ifstream romFile;
	romFile.open(fileName, std::ios::binary);
	if (romFile.fail())
	{
		printf("Error opening file!\n");
		return false;
	}

	BYTE ptr = 0x0100;
	for (ptr; ptr < 0x14F; ptr++)
	{
		char nxtByte;
		romFile.read(&nxtByte, 1);
		memory[ptr] = nxtByte;
	}

	for (ptr; ptr < 0x3FFF; ptr++)
	{
		char nxtByte;
		romFile.read(&nxtByte, 1);
		memory[ptr] = nxtByte;
	}

	return true;
}

void MMU::WriteByte(WORD addr, BYTE val)
{

	if (addr == 0xFF50)
	{
		bootromEnabled == false;
	}
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

void MMU::WriteWordBellow(WORD addr, WORD val)
{
	//Stored least significant first to emulate little-endianness
	memory[addr - 2] = (BYTE)(val);

	//Right shifted 8 bits to ignore the least significant byte
	//EG. 2000 = 00001011 1110000
	// 00001011 11100000 >> 8 = 11100000 00000000
	//Cast to byte and we store 11100000
	memory[addr - 1] = (BYTE)(val >> 8);
}

BYTE MMU::ReadByte(WORD addr)
{
	if (addr < 0x100)
	{
		if (bootromEnabled)
		{
			return BootstrapRom[addr];
		}
		else
		{
			return memory[addr];
		}
	}

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

WORD MMU::ReadWordBellow(WORD addr)
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
	return BytesToWord(memory[addr - 2], memory[addr - 1]);
}

