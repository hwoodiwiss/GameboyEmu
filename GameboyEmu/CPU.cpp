#include "CPU.h"

BYTE f_Zero = 0x80;
BYTE f_Subtract = 0x40;
BYTE f_HalfCarry = 0x20;
BYTE f_Carry = 0x10;

CPU::CPU(MMU *mmu)
{
	A, F, B, C, D, E, H, L = 0;
	_mmu = mmu;
	_PC = 0;
	//instruction = new Instruction("ADD", [](BYTE &A, BYTE x, BYTE y) {A = x + y; });

	RegInstruction(0x0020, 0, &CPU::JR_NZ_0x20);
	RegInstruction(0x0021, 0, &CPU::LD_HL_0x21);
	RegInstruction(0x0031, 0, &CPU::LD_SP_0x31);
	RegInstruction(0x0032, 0, &CPU::LD_HL_MINUS_0x32);
	RegInstruction(0x0066, 0, &CPU::LD_HL_H_0x66);
	RegInstruction(0x009F, 0, &CPU::SBC_A_A_0x9F);
	RegInstruction(0x00AF, 0, &CPU::XOR_A_0xAF);
	RegInstruction(0x00CB, 0, &CPU::PREFIX_0xCB);
	RegInstruction(0x00CC, 0, &CPU::CALL_Z_0xCC);
	RegInstruction(0x00CE, 0, &CPU::ADC_A_0xCE);
	RegInstruction(0x0020, 0, &CPU::JR_NZ_0x20);

	//CB Page instructions
	RegInstruction(0xCB7C, 0, &CPU::BIT_H_7_0xCB7C);


}

void CPU::RegInstruction(WORD opCode, BYTE duration, InstructionFunc function)
{
	BYTE lowPart = (opCode >> 8);
	if (lowPart == 0xCB)
	{
		Instruction op;
		op.opCode = (BYTE)opCode;
		op.duration = duration;
		op.function = function;
		cbPageInstructions[(BYTE)opCode] = op;
	}
	else
	{
		Instruction op;
		op.opCode = (BYTE)opCode;
		op.duration = duration;
		op.function = function;
		instructions[(BYTE)opCode] = op;
	}
}

CPU::~CPU()
{
	_mmu->~MMU();
}

void CPU::Boot()
{
	_mmu->LoadBootstrap();

}

void CPU::Tick()
{
	BYTE opCode = _mmu->ReadByte(_PC);

	Instruction op = instructions[opCode];
	
	if (op.function == 0x0000000000000000)
	{
		throw "Operation not implemented!\nTalk to Hugo.\n";
	}
	(*this.*op.function)();

	_PC++;
}

void CPU::IncSP()
{
	_SP -= 2;
}

void CPU::DecSP()
{
	_SP += 2;
}

WORD CPU::GetSP()
{
	return _SP;
}


void CPU::stackPush(WORD val)
{
	IncSP();
	_mmu->WriteWord(_SP, val);
}

WORD CPU::stackPop()
{
	WORD val = _mmu->ReadWord(_SP);
	DecSP();
	return val;
}


//Instructions Bellow, all 510 of them.

void CPU::JR_NZ_0x20()
{
	_PC++;
	BYTE operand = _mmu->ReadByte(_PC);
	if ((F & f_Zero) != f_Zero)
	{
		_PC += operand;
	}
}
void CPU::LD_HL_0x21()
{
	WORD operand = _mmu->ReadWord(_PC + 1);
	L = (BYTE)operand;
	H = (BYTE)operand >> 8;
	_PC += 2;
}

void CPU::LD_SP_0x31()
{
	WORD operand = _mmu->ReadWord(_PC + 1);
	_SP = operand;
	_PC += 2;
}

void CPU::LD_HL_MINUS_0x32()
{
	_mmu->WriteByte(BytesToWord(L, H), A);
}

void CPU::LD_HL_H_0x66()
{
	H = _mmu->ReadByte(BytesToWord(L, H));
}

void CPU::SBC_A_A_0x9F()
{
	A -= A;
	F |= f_Subtract;
	F |= f_Zero;
}

void CPU::XOR_A_0xAF()
{
	A ^= A;
	if (A == 0)
	{
		F |= f_Zero;
	}
	F &= ~f_Carry;
	F &= ~f_HalfCarry;
	F &= ~f_Subtract;
}



void CPU::PREFIX_0xCB()
{
	_PC++;
	BYTE opCode = _mmu->ReadByte(_PC);
	Instruction op = cbPageInstructions[opCode];
	(*this.*op.function)();
}

void CPU::CALL_Z_0xCC()
{
	WORD operand = _mmu->ReadWord(_PC + 1);
	if ((F & f_Zero) != f_Zero)
	{
		stackPush(_PC + 3);
		_PC = operand;
	}
	else
	{
		_PC++;
		_PC++;
	}
}

void CPU::ADC_A_0xCE()
{
	BYTE operand = _mmu->ReadByte(_PC + 1);
	if ((A & 0x8) == 0x8)
	{
		F |= f_HalfCarry;
	}
	A = A + operand;
	if (A == 0)
	{
		F |= f_Zero;
	}
	_PC++;
}

//CB Page Instructions
void CPU::BIT_H_7_0xCB7C()
{
	BYTE bitmask = 0x7F;
	F ^= ((H | bitmask) & 0x80);
	F &= ~f_Subtract;
	F |= f_HalfCarry;
}