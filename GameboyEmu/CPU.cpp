#include "CPU.h"

BYTE f_Zero = 0x80;
BYTE f_Subtract = 0x40;
BYTE f_HalfCarry = 0x20;
BYTE f_Carry = 0x10;

CPU::CPU(MMU *mmu)
{
	A, F, B, C, D, E, H, L = 0;
	_mmu = mmu;
	//instruction = new Instruction("ADD", [](BYTE &A, BYTE x, BYTE y) {A = x + y; });
}

CPU::~CPU()
{
	_mmu->~MMU();
}

void CPU::Boot()
{
	_PC = 0x103;
	
}

void CPU::Tick()
{
	BYTE opCode = _mmu->ReadByte(_PC);

	if (opCode == 0xCE)
	{
		BYTE operand = _mmu->ReadByte(_PC + 1);
		A = operand;
		_PC++;
	}
	else if (opCode == 0x66)
	{
		H = _mmu->ReadByte(BytesToWord(L, H));
		if (H == 0x00);
		{
			F |= f_Zero;
		}
	}
	else if (opCode == 0xCC)
	{
		WORD operand = _mmu->ReadWord(_PC + 1);
		if (F & f_Zero)
		{
			_SP = _PC + 3;
			_PC = operand - 1;
		}
		else
		{
			_PC++;
			_PC++;
		}
	}

	_PC++;
}
