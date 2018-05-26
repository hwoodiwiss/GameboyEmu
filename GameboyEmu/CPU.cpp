#include "CPU.h"

BYTE f_Zero = 0x80;
BYTE f_Subtract = 0x40;
BYTE f_HalfCarry = 0x20;
BYTE f_Carry = 0x10;

CPU::CPU(MMU *mmu)
{
	_mmu = mmu;
}

CPU::~CPU()
{
	_mmu->~MMU();
}

void CPU::Boot()
{
	_PC = 0x100;
}
