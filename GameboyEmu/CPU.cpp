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
	clock = 0;
	noInc = true;
	//instruction = new Instruction("ADD", [](BYTE &A, BYTE x, BYTE y) {A = x + y; });
	RegInstruction(0x0000, &CPU::NOP_0x00);
	RegInstruction(0x0001, &CPU::LD_BC_d16_0x01);
	RegInstruction(0x0002, &CPU::LD_pBC_A_0x02);
	RegInstruction(0x0003, &CPU::INC_BC_0x03);
	RegInstruction(0x0004, &CPU::INC_B_0x04);
	RegInstruction(0x0005, &CPU::DEC_B_0x05);
	RegInstruction(0x0006, &CPU::LD_B_d8_0x06);
	RegInstruction(0x0007, &CPU::RLCA_0x07);
	RegInstruction(0x0008, &CPU::LD_pd16_SP_0x08);
	RegInstruction(0x0009, &CPU::ADD_HL_BC_0x09);
	RegInstruction(0x000A, &CPU::LD_A_pBC_0x0A);
	RegInstruction(0x000B, &CPU::DEC_BC_0x0B);
	RegInstruction(0x000C, &CPU::INC_C_0x0C);
	RegInstruction(0x000D, &CPU::DEC_C_0x0D);
	RegInstruction(0x000E, &CPU::LD_C_d8_0x0E);
	RegInstruction(0x0010, &CPU::STOP_0x10);
	RegInstruction(0x0011, &CPU::LD_DE_d16_0x11);
	RegInstruction(0x0020, &CPU::JR_NZ_0x20);
	RegInstruction(0x0021, &CPU::LD_HL_0x21);
	RegInstruction(0x0031, &CPU::LD_SP_0x31);
	RegInstruction(0x0032, &CPU::LD_HL_MINUS_0x32);
	RegInstruction(0x003E, &CPU::LD_A_d8_0x3E);
	RegInstruction(0x004F, &CPU::LD_C_A_0x4F);
	RegInstruction(0x0066, &CPU::LD_HL_H_0x66);
	RegInstruction(0x0077, &CPU::LD_pHL_A_0x77);
	RegInstruction(0x0080, &CPU::ADD_A_B_0x80);
	RegInstruction(0x009F, &CPU::SBC_A_A_0x9F);
	RegInstruction(0x00AF, &CPU::XOR_A_0xAF);
	RegInstruction(0x00C5, &CPU::PUSH_BC_0xC5);
	RegInstruction(0x00CB, &CPU::PREFIX_0xCB);
	RegInstruction(0x00CC, &CPU::CALL_Z_0xCC);
	RegInstruction(0x00CD, &CPU::CALL_d16_0xCD);
	RegInstruction(0x00CE, &CPU::ADC_A_0xCE);
	RegInstruction(0x00E0, &CPU::LDH_pa8_A_0xE0);
	RegInstruction(0x00E2, &CPU::POP_HL_0xE2);
	RegInstruction(0x0020, &CPU::JR_NZ_0x20);

	//CB Page instructions
	RegInstruction(0xCB11, &CPU::RL_C_0xCB11);
	RegInstruction(0xCB7C, &CPU::BIT_H_7_0xCB7C);
	
}

void CPU::RegInstruction(WORD opCode, InstructionFunc function)
{
	BYTE lowPart = (opCode >> 8);
	if (lowPart == 0xCB)
	{
		Instruction op;
		op.opCode = (BYTE)opCode;
		op.function = function;
		cbPageInstructions[(BYTE)opCode] = op;
	}
	else
	{
		Instruction op;
		op.opCode = (BYTE)opCode;
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
	if (!noInc)
	{
		_PC++;
	}

	BYTE opCode = _mmu->ReadByte(_PC);

	Instruction op = instructions[opCode];
	noInc = false;

	if (op.function == 0)
	{
		throw opCode;
	}
	(*this.*op.function)();

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


//Instructions Bellow, all 510 of them. :'(
void CPU::NOP_0x00()
{
	clock += 4;
}

void CPU::LD_BC_d16_0x01()
{
	clock += 12;
	WORD operand = _mmu->ReadWord(_PC + 1);
	C = (BYTE)operand;
	B = (BYTE)operand >> 8;
	_PC += 2;
}

void CPU::LD_pBC_A_0x02()
{
	clock += 8;
	_mmu->WriteByte(BytesToWord(C, B), A);
}

void CPU::INC_BC_0x03()
{
	clock += 8;
	WORD val = BytesToWord(C, B);
	val++;
	B = (BYTE)val >> 8;
	C = (BYTE)val;
}

void CPU::INC_B_0x04()
{
	clock += 4;
	if ((B & 0xF) == 0xF)
	{
		F |= f_HalfCarry;
	}
	B++;
	if (B == 0)
	{
		F |= f_Zero;
	}
	F &= ~f_Subtract;
}

void CPU::DEC_B_0x05()
{
	clock += 4;
	if ((B & 0xF) == 0xF)
	{
		F |= f_HalfCarry;
	}
	B--;
	if (B == 0)
	{
		F |= f_Zero;
	}
	F |= f_Subtract;
}

void CPU::LD_B_d8_0x06()
{
	clock += 8;
	BYTE operand = _mmu->ReadByte(_PC + 1);
	B = operand;
	_PC++;
}

void CPU::RLCA_0x07()
{
	clock += 4;

	WORD tmp = (WORD)((A << 1));
	A = ((BYTE)tmp | (BYTE) tmp >> 8);
	BYTE flag = tmp >> 8;
	flag = flag << 4;
	
	F ^= ((flag | 0xEF) & 0x10);
	F &= ~f_Subtract;
	F &= ~f_HalfCarry;
	F &= ~f_Zero;
}

void CPU::LD_pd16_SP_0x08()
{
	clock += 20;
	WORD operand = _mmu->ReadWord(_PC + 1);
	_mmu->WriteWord(operand, _SP);
	_PC++;
	_PC++;
}

void CPU::ADD_HL_BC_0x09()
{
	clock += 8;
	WORD operand = BytesToWord(C, B);
	WORD target = BytesToWord(L, H);
	WORD result = operand + target;

	L = (BYTE)result;
	H = (BYTE)result >> 8;

	if ((((operand >> 8) & 0xF) + ((target >> 8) & 0xF)) & 0x10)
	{
		F |= f_HalfCarry;
	}
	if ((operand & 0x80) && (target & 0x80))
	{
		F |= f_Carry;
	}
	F &= ~f_Subtract;
}

void CPU::LD_A_pBC_0x0A()
{
	clock += 8;
	WORD addr = BytesToWord(C, B);
	A = _mmu->ReadByte(addr);
}

void CPU::DEC_BC_0x0B()
{
	clock += 8;
	WORD val = BytesToWord(C, B);
	val--;
	WordToBytes(C, B, val);
}

void CPU::INC_C_0x0C()
{
	clock += 4;
	if ((C & 0xF) == 0xF)
	{
		F |= f_HalfCarry;
	}
	C++;
	if (C == 0)
	{
		F |= f_Zero;
	}
	F &= ~f_Subtract;
}

void CPU::DEC_C_0x0D()
{
	clock += 4;
	if ((C & 0xF) == 0xF)
	{
		F |= f_HalfCarry;
	}
	C--;
	if (C == 0)
	{
		F |= f_Zero;
	}
	F |= f_Subtract;
}

void CPU::LD_C_d8_0x0E()
{
	clock += 8;
	BYTE operand = _mmu->ReadWord(_PC + 1);
	C = (BYTE)operand;
	_PC ++;
}

void CPU::RRCA_0x0F()
{
}

void CPU::STOP_0x10()
{

}

void CPU::LD_DE_d16_0x11()
{
	clock += 12;
	WORD operand = _mmu->ReadWord(_PC + 1);
	WordToBytes(E, D, operand);
	_PC += 2;
}

void CPU::JR_NZ_0x20()
{
	_PC++;
	SBYTE operand = _mmu->ReadByte(_PC);
	if ((F & f_Zero) != f_Zero)
	{
		_PC += operand;
		noInc = true;
		clock += 12;
	}
	else
	{
		clock += 8;
	}
}
void CPU::LD_HL_0x21()
{
	WORD operand = _mmu->ReadWord(_PC + 1);
	L = (BYTE)operand;
	H = (BYTE)operand >> 8;
	clock += 12;
	_PC += 2;
}

void CPU::LD_SP_0x31()
{
	WORD operand = _mmu->ReadWord(_PC + 1);
	_SP = operand;
	clock += 12;
	_PC += 2;
}

void CPU::LD_HL_MINUS_0x32()
{
	WORD val = BytesToWord(L, H);
	val++;
	WordToBytes(L, H, val);
	_mmu->WriteByte(BytesToWord(L, H), A);
	clock += 8;
}

void CPU::LD_A_d8_0x3E()
{
	clock += 8;
	BYTE operand = _mmu->ReadByte(_PC + 1);
	A = operand;
	_PC++;
}

void CPU::LD_C_A_0x4F()
{
	clock += 4;
	C = A;
}

void CPU::LD_HL_H_0x66()
{
	H = _mmu->ReadByte(BytesToWord(L, H));
	clock += 8;
}

void CPU::LD_pHL_A_0x77()
{
	clock += 8;
	_mmu->WriteByte(BytesToWord(L, H), A);
}

void CPU::ADD_A_B_0x80()
{
	clock += 4;
	WORD res = A + B;

	if ((((B >> 8) & 0xF) + ((A >> 8) & 0xF)) & 0x10)
	{
		F |= f_HalfCarry;
	}
	if (res & 0x100)
	{
		F |= f_Carry;
	}
	if (res == 0)
	{
		F |= f_Zero;
	}
	F &= ~f_Subtract;
	A = (BYTE)res;
}

void CPU::SBC_A_A_0x9F()
{
	A -= A;
	F |= f_Subtract;
	F |= f_Zero;
	clock += 4;
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
	clock += 4;
}

void CPU::PUSH_BC_0xC5()
{
	clock += 16;
	stackPush(BytesToWord(C, B));
}



void CPU::PREFIX_0xCB()
{
	_PC++;
	BYTE opCode = _mmu->ReadByte(_PC);
	clock += 4;
	Instruction op = cbPageInstructions[opCode];
	(*this.*op.function)();
}

void CPU::CALL_Z_0xCC()
{
	WORD operand = _mmu->ReadWord(_PC + 1);
	if ((F & f_Zero) != f_Zero)
	{
		clock += 24;
		stackPush(_PC + 2);
		noInc = true;
		_PC = operand;
	}
	else
	{
		clock += 12;
		_PC++;
		_PC++;
	}
}

void CPU::CALL_d16_0xCD()
{
	clock += 24;
	WORD operand = _mmu->ReadWord(_PC + 1);
	stackPush(_PC + 2);
	noInc = true;
	_PC = operand;
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
	clock += 8;
	_PC++;
}

void CPU::POP_HL_0xE2()
{
	clock += 12;
	WORD var = _mmu->ReadWord(_SP);
	DecSP();
	WordToBytes(L, H, var);
}

void CPU::LDH_pa8_A_0xE0()
{
	clock += 12;
	BYTE operand = 0xFF00 + _mmu->ReadByte(_PC + 1);

	_PC++;
}

//CB Page Instructions
void CPU::BIT_H_7_0xCB7C()
{
	BYTE bitmask = 0x7F;
	F ^= ((H | bitmask) & 0x80);
	F &= ~f_Subtract;
	F |= f_HalfCarry;
	clock += 8;
}

void CPU::RL_C_0xCB11()
{
	clock += 4;

	WORD tmp = (WORD)((A << 1) | ((F & f_Carry) >> 4));
	A = (BYTE)tmp;
	BYTE flag = tmp >> 8;
	flag = flag << 4;

	F ^= ((flag | 0xEF) & 0x10);
	F &= ~f_Subtract;
	F &= ~f_HalfCarry;
	F &= ~f_Zero;
}
