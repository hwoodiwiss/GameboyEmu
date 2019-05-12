#include "CPU.h"

BYTE f_Zero = 0x80;
BYTE f_Subtract = 0x40;
BYTE f_HalfCarry = 0x20;
BYTE f_Carry = 0x10;

CPU::CPU(MMU* mmu)
{
	A = 0, F = 0, B = 0, C = 0, D = 0, E = 0, H = 0, L = 0;
	IF = 0, IME = 0, _SP = 0;
	_mmu = mmu;
	_PC = 0;
	clock = 0;
	noInc = true;
	HALT = false;

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
	RegInstruction(0x0012, &CPU::LD_pDE_A_0x12);
	RegInstruction(0x0013, &CPU::INC_DE_0x13);
	RegInstruction(0x0017, &CPU::RLA_0x17);
	RegInstruction(0x001A, &CPU::AND_C_0x1A);
	RegInstruction(0x001E, &CPU::LD_E_d8_0x1E);
	RegInstruction(0x0020, &CPU::JR_NZ_0x20);
	RegInstruction(0x0021, &CPU::LD_HL_0x21);
	RegInstruction(0x0022, &CPU::LD_HL_PLUS_0x22);
	RegInstruction(0x0023, &CPU::INC_HL_0x23);
	RegInstruction(0x0028, &CPU::JR_Z_r8_0x28);
	RegInstruction(0x0031, &CPU::LD_SP_0x31);
	RegInstruction(0x0032, &CPU::LD_HL_MINUS_0x32);
	RegInstruction(0x003C, &CPU::INC_A_0x3C);
	RegInstruction(0x003D, &CPU::DEC_A_0x3D);
	RegInstruction(0x003E, &CPU::LD_A_d8_0x3E);
	RegInstruction(0x004F, &CPU::LD_C_A_0x4F);
	RegInstruction(0x0057, &CPU::LD_D_A_0x57);
	RegInstruction(0x0066, &CPU::LD_HL_H_0x66);
	RegInstruction(0x0067, &CPU::LD_H_A_0x67);
	RegInstruction(0x0077, &CPU::LD_pHL_A_0x77);
	RegInstruction(0x007B, &CPU::LD_A_E_0x7B);
	RegInstruction(0x0080, &CPU::ADD_A_B_0x80);
	RegInstruction(0x009F, &CPU::SBC_A_A_0x9F);
	RegInstruction(0x00AF, &CPU::XOR_A_0xAF);
	RegInstruction(0x00C1, &CPU::RET_NZ_0xC1);
	RegInstruction(0x00C5, &CPU::PUSH_BC_0xC5);
	RegInstruction(0x00CB, &CPU::PREFIX_0xCB);
	RegInstruction(0x00CC, &CPU::CALL_Z_0xCC);
	RegInstruction(0x00CD, &CPU::CALL_d16_0xCD);
	RegInstruction(0x00CE, &CPU::ADC_A_0xCE);
	RegInstruction(0x00E0, &CPU::LDH_pa8_A_0xE0);
	RegInstruction(0x00E1, &CPU::POP_HL_0xE1);
	RegInstruction(0x00E2, &CPU::LD_pC_A_0xE2);
	RegInstruction(0x00EA, &CPU::LD_pa16_A_0xEA);
	RegInstruction(0x00F0, &CPU::LDH_0xF0);
	RegInstruction(0x00F3, &CPU::DI_0xF3);
	RegInstruction(0x00FE, &CPU::CP_d8_0xFE);

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
#if(_DEBUG)
	prevOp = opCode;
#endif
	Instruction op = instructions[opCode];
	noInc = false;

	if (op.function == 0)
	{
		throw BytesToWord(opCode, _PC);
	}
	(*this.*op.function)();

}

#if(_DEBUG)
void CPU::DrawState()
{
	ClearScreen();
	printf("WARNING! Debug output slows operation SIGNIFICANTLY.\nGameboy assembly loops are painfully slow.\nUse release build to test at real/near real performance.\n");
	printf("A = D:%u H:%02X | F = D:%u H:%02X\nB = D:%u H:%02X | C = D:%u H: %02X\nD = D:%u H:%02X | E = D:%u H:%02X\nH = D:%u H:%02X | L = D:%u H:%02X\n_______________________________\n    PC = D:%u H:%04X\n    SP = D:%u H:%04X\n    BC = D:%u H:%04X\n    DE = D:%u H:%04X\n    HL = D:%u H:%04X\n\n\nLast Op: %04X\n", A, A, F, F, B, B, C, C, D, D, E, E, H, H, L, L, _PC, _PC, _SP, _SP, BytesToWord(C, B), BytesToWord(C, B), BytesToWord(E, D), BytesToWord(E, D), BytesToWord(L, H), BytesToWord(L, H), prevOp);
	Sleep(1);
}
#endif

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
	_mmu->WriteWord(_SP, val);
	IncSP();
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
	LD(&C, &B, operand);
	_PC += 2;
}

void CPU::LD_pBC_A_0x02()
{
	clock += 8;
	LD(BytesToWord(C, B), A);
}

void CPU::INC_BC_0x03()
{
	clock += 8;
	INC(&C, &B);
}

void CPU::INC_B_0x04()
{
	clock += 4;
	INC(&B);
}

void CPU::DEC_B_0x05()
{
	clock += 4;
	DEC(&B);
}

void CPU::LD_B_d8_0x06()
{
	clock += 8;
	BYTE operand = _mmu->ReadByte(_PC + 1);
	LD(&B, operand);
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
	ADD(BytesToWord(C, B));
}

void CPU::LD_A_pBC_0x0A()
{
	clock += 8;
	WORD addr = BytesToWord(C, B);
	LD(&A, _mmu->ReadByte(addr));
}

void CPU::DEC_BC_0x0B()
{
	clock += 8;
	DEC(&C, &B);
}

void CPU::INC_C_0x0C()
{
	clock += 4;
	INC(&C);
}

void CPU::DEC_C_0x0D()
{
	clock += 4;
	DEC(&C);
}

void CPU::LD_C_d8_0x0E()
{
	clock += 8;
	BYTE operand = _mmu->ReadByte(_PC + 1);
	LD(&C, operand);
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
	LD(&E, &D, _mmu->ReadWord(_PC + 1));
	_PC += 2;
}

void CPU::LD_pDE_A_0x12()
{
	clock += 8;
	LD(BytesToWord(E, D), A);
}

void CPU::INC_DE_0x13()
{
	clock += 8;
	INC(&E, &D);
}

void CPU::RLA_0x17()
{
	clock += 4;

	WORD tmp = (WORD)((A << 1) | ((F & f_Carry) >> 4));
	A = (BYTE)tmp;
	BYTE flag = (tmp >> 8);
	flag = flag << 4;

	F ^= ((flag | 0xEF) & 0x10);
	F &= ~f_Subtract;
	F &= ~f_HalfCarry;
	F &= ~f_Zero;
}

void CPU::AND_C_0x1A()
{
	clock += 4;
	A = A & C;
	if (A == 0)
	{
		F |= f_Zero;
	}
	else
	{
		F &= ~f_Zero;
	}
	F &= ~(f_Carry | f_Subtract);
}

void CPU::LD_E_d8_0x1E()
{
	clock += 8;
	LD(&E, _mmu->ReadByte(_PC + 1));
	_PC++;
}

void CPU::JR_NZ_0x20()
{
	SBYTE operand = _mmu->ReadByte(_PC + 1);
	bool cnd = ((F & f_Zero) != f_Zero);
	JR(cnd, operand);
}
void CPU::LD_HL_0x21()
{
	clock += 12;
	WORD operand = _mmu->ReadWord(_PC + 1);
	LD(&L, &H, operand);
	_PC += 2;
}

void CPU::LD_HL_PLUS_0x22()
{
	WORD val = BytesToWord(L, H);
	val++;
	WordToBytes(L, H, val);
	_mmu->WriteByte(BytesToWord(L, H), A);
	clock += 8;
}

void CPU::INC_HL_0x23()
{
	clock += 8;
	INC(&L, &H);
}

void CPU::JR_Z_r8_0x28()
{
	SBYTE operand = _mmu->ReadByte(_PC + 1);
	JR(((F & f_Zero) == f_Zero), operand);
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
	val--;
	WordToBytes(L, H, val);
	_mmu->WriteByte(BytesToWord(L, H), A);
	clock += 8;
}

void CPU::INC_A_0x3C()
{
	clock += 4;
	INC(&A);
}

void CPU::DEC_A_0x3D()
{
	clock += 4;
	DEC(&A);
}

void CPU::LD_A_d8_0x3E()
{
	clock += 8;
	LD(&A, _mmu->ReadByte(_PC + 1));
	_PC++;
}

void CPU::LD_C_A_0x4F()
{
	clock += 4;
	LD(&C, A);
}

void CPU::LD_D_A_0x57()
{
	clock += 4;
	LD(&D, A);
}

void CPU::LD_HL_H_0x66()
{
	LD(&H, _mmu->ReadByte(BytesToWord(L, H)));
	clock += 8;
}

void CPU::LD_H_A_0x67()
{
	clock += 4;
	LD(&H, A);
}

void CPU::LD_pHL_A_0x77()
{
	clock += 8;
	LD(BytesToWord(L, H), A);
}

void CPU::LD_A_E_0x7B()
{
	clock += 4;
	LD(&A, E);
}

void CPU::ADD_A_B_0x80()
{
	clock += 4;
	ADD(B);
}

void CPU::SBC_A_A_0x9F()
{
	SBC(&A, A);
	clock += 4;
}

void CPU::XOR_A_0xAF()
{
	XOR(A);
	clock += 4;
}

void CPU::RET_NZ_0xC1()
{
	RET(!((F & f_Zero) == f_Zero));
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
#if(_DEBUG)
	prevOp = BytesToWord(opCode, prevOp);
#endif
	Instruction op = cbPageInstructions[opCode];
	(*this.*op.function)();
}

void CPU::CALL_Z_0xCC()
{
	WORD operand = _mmu->ReadWord(_PC + 1);
	CALL(((F & f_Zero) != f_Zero), operand);
}

void CPU::CALL_d16_0xCD()
{
	WORD operand = _mmu->ReadWord(_PC + 1);
	CALL(true, operand);
}

void CPU::ADC_A_0xCE()
{
	BYTE operand = _mmu->ReadByte(_PC + 1);
	ADC(&A, operand);
}

void CPU::LDH_pa8_A_0xE0()
{
	clock += 12;
	WORD operand = 0xFF00 + _mmu->ReadByte(_PC + 1);
	_mmu->WriteByte(operand, A);
	_PC++;
}

void CPU::LDH_0xF0()
{
	clock += 12;
	WORD operand = 0xFF00 + _mmu->ReadByte(_PC + 1);
	A = _mmu->ReadByte(operand);
	_PC++;
}

void CPU::DI_0xF3()
{
	clock += 4;
	_mmu->WriteByte(0xFFFF, 0);
}

void CPU::POP_HL_0xE1()
{
	POP(&L, &H);
}

void CPU::LD_pC_A_0xE2()
{
	clock += 8;
	WORD addr = 0xFF00 + C;
	_mmu->WriteByte(addr, A);
}

void CPU::LD_pa16_A_0xEA()
{
	clock += 16;
	WORD addr = _mmu->ReadWord(_PC + 1);
	_mmu->WriteByte(addr, A);
	_PC += 2;
}

void CPU::CP_d8_0xFE()
{
	BYTE operand = _mmu->ReadByte(_PC + 1);
	BYTE valA = A;
	valA -= operand;
	
	if (A == operand)
	{
		F |= f_Zero;
	}
	else
	{
		F &= ~f_Zero;
	}

	if (A < operand)
	{
		F |= f_Carry;
	}
	else
	{
		F &= ~f_Carry;
	}

	if ((((A & 0xF) - (operand & 0xF)) < 0))
	{
		F |= f_HalfCarry;
	}
	else
	{
		F &= ~ f_HalfCarry;
	}
	if (valA == 0)
	{
		F |= f_Zero;
	}
	F |= f_Subtract;
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
	clock += 8;

	WORD tmp = (WORD)((C << 1) | ((F & f_Carry) >> 4));
	C = (BYTE)tmp;
	BYTE flag = (tmp >> 8);
	flag = flag << 4;

	F ^= ((flag | 0xEF) & 0x10);
	F &= ~f_Subtract;
	F &= ~f_HalfCarry;
	F &= ~f_Zero;
}

void CPU::LD(BYTE * _register, BYTE operand)
{
	(*_register) = operand;
}

void CPU::LD(BYTE * regLow, BYTE * regHigh, WORD operand)
{
	(*regLow) = (BYTE)operand;
	(*regHigh) = (BYTE)(operand >> 8);
}

void CPU::LD(WORD _address, BYTE operand)
{
	_mmu->WriteByte(_address, operand);
}

void CPU::ADD(BYTE operand)
{
	WORD res = A + operand;

	if ((((operand >> 8) & 0xF) + ((A >> 8) & 0xF)) & 0x10)
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

void CPU::ADD(WORD operand)
{
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

void CPU::ADC(BYTE * _register, BYTE* operand)
{
	if (((*_register) & 0x8) == 0x8)
	{
		F |= f_HalfCarry;
	}
	(*_register) = (*_register) + (*operand);
	if ((*_register) == 0)
	{
		F |= f_Zero;
	}
	clock += 4;
	_PC++;
}

void CPU::ADC(BYTE * _register, BYTE operand)
{
	if (((*_register) & 0x8) == 0x8)
	{
		F |= f_HalfCarry;
	}
	(*_register) = (*_register) + operand;
	if ((*_register) == 0)
	{
		F |= f_Zero;
	}
	clock += 8;
	_PC++;
}

void CPU::ADC(BYTE * _register, WORD operand)
{
}

void CPU::SUB(BYTE operand)
{
}

void CPU::SUB(WORD operand)
{
}

void CPU::SBC(BYTE* _register, BYTE operand)
{
	if (((*_register) & 0x10) == 0x10)
	{
		F |= f_HalfCarry;
	}
	(*_register) -= operand;
	if ((*_register) == 0)
	{
		F |= f_Zero;
	}
	F |= f_Subtract;
}

void CPU::SBC(BYTE* _register, WORD operand)
{
}

void CPU::INC(BYTE * _register)
{
	if (((*_register) & 0xF) == 0xF)
	{
		F |= f_HalfCarry;
	}
	(*_register)++;
	if ((*_register) == 0)
	{
		F |= f_Zero;
	}
	F &= ~f_Subtract;
}

void CPU::INC(BYTE * regLow, BYTE * regHigh)
{
	WORD val = BytesToWord((*regLow), (*regHigh));
	val++;
	WordToBytes((*regLow), (*regHigh), val);
}

void CPU::DEC(BYTE * _register)
{
	if (((*_register) & 0x10) == 0x10)
	{
		F |= f_HalfCarry;
	}
	(*_register)--;
	if ((*_register) == 0)
	{
		F |= f_Zero;
	}
	F |= f_Subtract;
}

void CPU::DEC(BYTE * regLow, BYTE * regHigh)
{
	WORD val = BytesToWord((*regLow), (*regHigh));
	val--;
	WordToBytes((*regLow), (*regHigh), val);
}

void CPU::BIT(BYTE * _register, BYTE bit)
{
}

void CPU::JR(bool condition, SBYTE offset)
{
	if (condition)
	{
		_PC++;
		clock += 12;
		_PC += offset;
		noInc = true;
	}
	else
	{
		_PC++;
		clock += 8;
	}
}

void CPU::XOR(BYTE operand)
{
	A ^= operand;
	if (A == 0)
	{
		F |= f_Zero;
	}
	F &= ~f_Carry;
	F &= ~f_HalfCarry;
	F &= ~f_Subtract;
}

void CPU::RET()
{
	_PC = stackPop();
	clock += 16;
}

void CPU::RET(bool condition)
{
	if (condition)
	{
		_PC = stackPop();
		clock += 20;
	}
	else
	{
		clock += 8;
	}
}

void CPU::PUSH(WORD address)
{
	stackPush(address);
	clock+=16;
}

void CPU::POP(BYTE * regLow, BYTE * regHigh)
{
	clock += 12;
	WORD var = _mmu->ReadWord(_SP);
	DecSP();
	WordToBytes((*regLow), (*regHigh), var);
}

void CPU::CALL(bool condition, WORD addr)
{
	if (condition)
	{
		clock += 24;
		stackPush(_PC + 2);
		noInc = true;
		_PC = addr;
	}
	else
	{
		clock += 12;
		_PC++;
		_PC++;
	}
}
