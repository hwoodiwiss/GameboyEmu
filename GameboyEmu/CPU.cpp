#include "CPU.h"

BYTE f_Zero = 0x80;
BYTE f_Subtract = 0x40;
BYTE f_HalfCarry = 0x20;
BYTE f_Carry = 0x10;

CPU::CPU(MMU* mmu)
{
	AF = 0, BC = 0, DE = 0, HL = 0;
	IF = 0, IME = 0, _SP = 0;
	_mmu = mmu;
	_PC = 0;
	clock = 0;
	noInc = true;
	HALT = false;
	
	RegInstruction(0x0000, &CPU::NOP_0x00, 1, 4);
	RegInstruction(0x0001, &CPU::LD_BC_d16_0x01, 3, 12);
	RegInstruction(0x0002, &CPU::LD_pBC_A_0x02, 1, 8);
	RegInstruction(0x0003, &CPU::INC_BC_0x03, 1, 8);
	RegInstruction(0x0004, &CPU::INC_B_0x04, 1, 4);
	RegInstruction(0x0005, &CPU::DEC_B_0x05, 1, 4);
	RegInstruction(0x0006, &CPU::LD_B_d8_0x06, 2, 8);
	RegInstruction(0x0007, &CPU::RLCA_0x07, 1, 4);
	RegInstruction(0x0008, &CPU::LD_pd16_SP_0x08, 3, 20);
	RegInstruction(0x0009, &CPU::ADD_HL_BC_0x09, 1, 8);
	RegInstruction(0x000A, &CPU::LD_A_pBC_0x0A, 1, 8);
	RegInstruction(0x000B, &CPU::DEC_BC_0x0B, 1, 8);
	RegInstruction(0x000C, &CPU::INC_C_0x0C, 1, 4);
	RegInstruction(0x000D, &CPU::DEC_C_0x0D, 1, 4);
	RegInstruction(0x000E, &CPU::LD_C_d8_0x0E, 2, 8);

	/*There was a hardware bug in the original GB and GBC where the Op directly after a STOP would be ignored.
	  Nintendo reccomeded just putting a NOP after, as it will be skipped anyway, although, I get the feeling
	  some crafty devs will have put ops here that they wanted skipped in the first iteration of a loop, that will
	  be the op that gets jumped too*/
	RegInstruction(0x0010, &CPU::STOP_0x10, 2, 4);
	RegInstruction(0x0011, &CPU::LD_DE_d16_0x11, 3, 12);
	RegInstruction(0x0012, &CPU::LD_pDE_A_0x12, 1, 8);
	RegInstruction(0x0013, &CPU::INC_DE_0x13, 1, 8);
	RegInstruction(0x0017, &CPU::RLA_0x17, 1, 4);
	RegInstruction(0x001A, &CPU::AND_C_0x1A, 1, 8);
	RegInstruction(0x001E, &CPU::LD_E_d8_0x1E, 2, 8);
	RegInstruction(0x0020, &CPU::JR_NZ_0x20, 2, 8);
	RegInstruction(0x0021, &CPU::LD_HL_0x21, 3, 16);
	RegInstruction(0x0022, &CPU::LD_HL_PLUS_0x22, 1, 8);
	RegInstruction(0x0023, &CPU::INC_HL_0x23, 1, 8);
	RegInstruction(0x0028, &CPU::JR_Z_r8_0x28, 2, 8);
	RegInstruction(0x0031, &CPU::LD_SP_0x31, 3, 12);
	RegInstruction(0x0032, &CPU::LD_HL_MINUS_0x32, 1, 8);
	RegInstruction(0x003C, &CPU::INC_A_0x3C, 1, 4);
	RegInstruction(0x003D, &CPU::DEC_A_0x3D, 1, 4);
	RegInstruction(0x003E, &CPU::LD_A_d8_0x3E, 2, 8);
	RegInstruction(0x004F, &CPU::LD_C_A_0x4F, 1, 4);
	RegInstruction(0x0057, &CPU::LD_D_A_0x57, 1, 4);
	RegInstruction(0x0066, &CPU::LD_HL_H_0x66, 1, 8);
	RegInstruction(0x0067, &CPU::LD_H_A_0x67, 1, 4);
	RegInstruction(0x0077, &CPU::LD_pHL_A_0x77, 1, 8);
	RegInstruction(0x007B, &CPU::LD_A_E_0x7B, 1, 4);
	RegInstruction(0x0080, &CPU::ADD_A_B_0x80, 1, 4);
	RegInstruction(0x009F, &CPU::SBC_A_A_0x9F, 1, 4);
	RegInstruction(0x00AF, &CPU::XOR_A_0xAF, 1, 4);
	RegInstruction(0x00C1, &CPU::RET_NZ_0xC1, 1, 12);
	RegInstruction(0x00C5, &CPU::PUSH_BC_0xC5, 1, 16);
	RegInstruction(0x00CB, &CPU::PREFIX_0xCB, 1, 4);
	RegInstruction(0x00CC, &CPU::CALL_Z_0xCC, 3, 12);
	RegInstruction(0x00CD, &CPU::CALL_d16_0xCD, 3, 24);
	RegInstruction(0x00CE, &CPU::ADC_A_0xCE, 2, 8);
	RegInstruction(0x00E0, &CPU::LDH_pa8_A_0xE0, 2, 12);
	RegInstruction(0x00E1, &CPU::POP_HL_0xE1, 1, 12);
	RegInstruction(0x00E2, &CPU::LD_pC_A_0xE2, 1, 8);
	RegInstruction(0x00EA, &CPU::LD_pa16_A_0xEA, 3, 16);
	RegInstruction(0x00F0, &CPU::LDH_0xF0, 2, 12);
	RegInstruction(0x00F3, &CPU::DI_0xF3, 1, 3);
	RegInstruction(0x00FE, &CPU::CP_d8_0xFE, 2, 8);

	//CB Page instructions
	RegInstruction(0xCB11, &CPU::RL_C_0xCB11, 1, 4);
	RegInstruction(0xCB7C, &CPU::BIT_H_7_0xCB7C, 1, 4);
	
}

void CPU::RegInstruction(WORD opCode, InstructionFunc function, BYTE instructionSize, BYTE instructionDuration )
{
	BYTE lowPart = (opCode >> 8);
	if (lowPart == 0xCB)
	{
		Instruction op;
		op.opCode = (BYTE)opCode;
		op.function = function;
		op.opDuration = instructionDuration;
		op.opSize = instructionSize;
		cbPageInstructions[(BYTE)opCode] = op;
	}
	else
	{
		Instruction op;
		op.opCode = (BYTE)opCode;
		op.function = function;
		op.opDuration = instructionDuration;
		op.opSize = instructionSize;
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

	prevOp = opCode;

	Instruction op = instructions[opCode];
	noInc = false;
	if (op.function == 0)
	{
		throw BytesToWord(opCode, _PC);
	}
	clock += op.opDuration;
	(*this.*op.function)();

	if(!noInc)
		_PC += op.opSize;
}

void CPU::ToggleDisplay()
{
	m_DrawState = !m_DrawState;
}

void CPU::DrawState()
{
	if (m_DrawState)
	{
		ClearScreen();
		printf("WARNING! Debug output slows operation SIGNIFICANTLY.\nGameboy assembly loops are painfully slow.\nUse release build to test at real/near real performance.\n");
		printf("A = D:%u H:%02X B:" BinStr " | F = D:%u H:%02X B:" BinStr "\nB = D:%u H:%02X B:" BinStr " | C = D:%u H: %02X B:" BinStr "\nD = D:%u H:%02X B:" BinStr \
			" | E = D:%u H:%02X B:" BinStr "\nH = D:%u H:%02X B:" BinStr " | L = D:%u H:%02X B:" BinStr "\n_______________________________\n    PC = D:%u H:%04X\n \
   SP = D:%u H:%04X\n    BC = D:%u H:%04X\n    DE = D:%u H:%04X\n    HL = D:%u H:%04X\n\n\nLast Op: %04X\n", A, A, ByteToBinString(A), F, F, ByteToBinString(F), B, B, ByteToBinString(B), C, C, ByteToBinString(C), D, D, ByteToBinString(D), E, E, ByteToBinString(E), H, H, ByteToBinString(H), L, L, ByteToBinString(L), _PC, _PC, _SP, _SP, BytesToWord(C, B), BytesToWord(C, B), BytesToWord(E, D), BytesToWord(E, D), BytesToWord(L, H), BytesToWord(L, H), prevOp);
		Sleep(1);
	}
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
	//Do nothing
}

void CPU::LD_BC_d16_0x01()
{
	WORD operand = _mmu->ReadWord(_PC + 1);
	LD(BC, operand);
}

void CPU::LD_pBC_A_0x02()
{
	LD(BC, A);
}

void CPU::INC_BC_0x03()
{
	INC(BC);
}

void CPU::INC_B_0x04()
{
	INC(B);
}

void CPU::DEC_B_0x05()
{
	DEC(B);
}

void CPU::LD_B_d8_0x06()
{
	BYTE operand = _mmu->ReadByte(_PC + 1);
	LD(B, operand);
}

void CPU::RLCA_0x07()
{
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
	WORD operand = _mmu->ReadWord(_PC + 1);
	_mmu->WriteWord(operand, _SP);
}

void CPU::ADD_HL_BC_0x09()
{
	ADD(BC);
}

void CPU::LD_A_pBC_0x0A()
{
	LD(A, _mmu->ReadByte(BC));
}

void CPU::DEC_BC_0x0B()
{
	DEC(BC);
}

void CPU::INC_C_0x0C()
{
	INC(C);
}

void CPU::DEC_C_0x0D()
{
	DEC(C);
}

void CPU::LD_C_d8_0x0E()
{
	BYTE operand = _mmu->ReadByte(_PC + 1);
	LD(C, operand);
}

void CPU::RRCA_0x0F()
{
}

void CPU::STOP_0x10()
{

}

void CPU::LD_DE_d16_0x11()
{
	LD(DE, _mmu->ReadWord(_PC + 1));
}

void CPU::LD_pDE_A_0x12()
{
	LD(DE, A);
}

void CPU::INC_DE_0x13()
{
	INC(DE);
}

void CPU::RLA_0x17()
{
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
	LD(E, _mmu->ReadByte(_PC + 1));
}

void CPU::JR_NZ_0x20()
{
	SBYTE operand = _mmu->ReadByte(_PC + 1);
	bool cnd = ((F & f_Zero) != f_Zero);
	JR(cnd, operand);
}
void CPU::LD_HL_0x21()
{
	WORD operand = _mmu->ReadWord(_PC + 1);
	LD(HL, operand);
}

void CPU::LD_HL_PLUS_0x22()
{
	LD(HL, A);
	HL++;
}

void CPU::INC_HL_0x23()
{
	INC(HL);
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
}

void CPU::LD_HL_MINUS_0x32()
{
	LD(HL, A);
	HL--;
}

void CPU::INC_A_0x3C()
{
	INC(A);
}

void CPU::DEC_A_0x3D()
{
	DEC(A);
}

void CPU::LD_A_d8_0x3E()
{
	LD(A, _mmu->ReadByte(_PC + 1));
}

void CPU::LD_C_A_0x4F()
{
	LD(C, A);
}

void CPU::LD_D_A_0x57()
{
	LD(D, A);
}

void CPU::LD_HL_H_0x66()
{
	LD(H, _mmu->ReadByte(HL));
}

void CPU::LD_H_A_0x67()
{
	LD(H, A);
}

void CPU::LD_pHL_A_0x77()
{
	LD(HL, A);
}

void CPU::LD_A_E_0x7B()
{
	LD(A, E);
}

void CPU::ADD_A_B_0x80()
{
	ADD(B);
}

void CPU::SBC_A_A_0x9F()
{
	SBC(A, A);
}

void CPU::XOR_A_0xAF()
{
	XOR(A);
}

void CPU::RET_NZ_0xC1()
{
	RET(!((F & f_Zero) == f_Zero));
}

void CPU::PUSH_BC_0xC5()
{
	stackPush(BC);
}



void CPU::PREFIX_0xCB()
{
	BYTE opCode = _mmu->ReadByte(_PC + 1);
	prevOp = BytesToWord(opCode, prevOp);
	Instruction op = cbPageInstructions[opCode];
	(*this.*op.function)();
	clock += op.opDuration;
	_PC += op.opSize;
}

void CPU::CALL_Z_0xCC()
{
	WORD operand = _mmu->ReadWord(_PC + 1);
	CALL(((F & f_Zero) != f_Zero), operand);
}

void CPU::CALL_d16_0xCD()
{
	WORD operand = _mmu->ReadWord(_PC + 1);
	CALL(operand);
}

void CPU::ADC_A_0xCE()
{
	BYTE operand = _mmu->ReadByte(_PC + 1);
	ADC(A, operand);
}

void CPU::LDH_pa8_A_0xE0()
{
	WORD operand = 0xFF00 + _mmu->ReadByte(_PC + 1);
	_mmu->WriteByte(operand, A);
}

void CPU::LDH_0xF0()
{
	WORD operand = 0xFF00 + _mmu->ReadByte(_PC + 1);
	A = _mmu->ReadByte(operand);
}

void CPU::DI_0xF3()
{
	clock += 4;
	_mmu->WriteByte(0xFFFF, 0);
}

void CPU::POP_HL_0xE1()
{
	POP(HL);
}

void CPU::LD_pC_A_0xE2()
{
	clock += 8;
	WORD addr = 0xFF00 + C;
	_mmu->WriteByte(addr, A);
}

void CPU::LD_pa16_A_0xEA()
{
	WORD addr = _mmu->ReadWord(_PC + 1);
	_mmu->WriteByte(addr, A);
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
}

//CB Page Instructions
void CPU::BIT_H_7_0xCB7C()
{
	BIT(H, 7);
}

void CPU::RL_C_0xCB11()
{
	WORD tmp = (WORD)((C << 1) | ((F & f_Carry) >> 4));
	C = (BYTE)tmp;
	BYTE flag = (tmp >> 8);
	flag = flag << 4;

	F ^= ((flag | 0xEF) & 0x10);
	F &= ~f_Subtract;
	F &= ~f_HalfCarry;
	F &= ~f_Zero;
}

void CPU::LD(BYTE& _register, BYTE operand)
{
	_register = operand;
}

void CPU::LD(WORD& _reg16, WORD operand)
{
	_reg16 = operand;
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
	WORD target = HL;

	WORD result = operand + target;

	HL = result;

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

void CPU::ADC(BYTE& _register, BYTE& operand)
{
	if ((_register & 0x8) == 0x8)
	{
		F |= f_HalfCarry;
	}
	_register = _register + operand;
	if (_register == 0)
	{
		F |= f_Zero;
	}
}

void CPU::ADC(BYTE& _register, WORD operand)
{
}

void CPU::BIT(BYTE& _register, BYTE nBit)
{
	BYTE nMask = ((BYTE)1 << (BYTE)nBit);
	if ((_register & nMask) == nMask)
	{
		F &= ~nMask;
	}
	else
	{
		F |= nMask;
	}
	F &= ~f_Subtract;
	F |= f_HalfCarry;
}

void CPU::SUB(BYTE operand)
{
}

void CPU::SUB(WORD operand)
{
}

void CPU::SBC(BYTE& _register, BYTE operand)
{
	if ((_register & 0x10) == 0x10)
	{
		F |= f_HalfCarry;
	}
	_register -= operand;
	if (_register == 0)
	{
		F |= f_Zero;
	}
	F |= f_Subtract;
}

void CPU::SBC(BYTE& _register, WORD operand)
{
}

void CPU::INC(BYTE& _register)
{
	if ((_register & 0xF) == 0xF)
	{
		F |= f_HalfCarry;
	}
	_register++;
	if (_register == 0)
	{
		F |= f_Zero;
	}
	F &= ~f_Subtract;
}

//Increment a 16-bit register value.
//TODO: Add soul-crushing flag manipulation
void CPU::INC(WORD& reg16)
{
	reg16++;
}

//8-bit decrement. Extra complex, as we have to decrement the value, and set all the required flags.
void CPU::DEC(BYTE& _register)
{
	if ((_register & 0x10) == 0x10)
	{
		F |= f_HalfCarry;
	}
	_register--;
	if (_register == 0)
	{
		F |= f_Zero;
	}
	F |= f_Subtract;
}

void CPU::DEC(WORD& reg16)
{
	reg16--;
}

void CPU::JR(bool condition, SBYTE offset)
{
	if (condition)
	{
		clock += 4; //Have to add to clock, as branching takes longer than not branching
		_PC += offset;
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

//Unconditional return
void CPU::RET()
{
	_PC = stackPop();
}

//Conditional return
void CPU::RET(bool condition)
{
	if (condition)
	{
		clock += 12; //Have to add to clock, as branching takes longer than not branching
		_PC = stackPop();
	}
}

void CPU::PUSH(WORD address)
{
	stackPush(address);
}

void CPU::POP(WORD& reg16)
{
	WORD var = _mmu->ReadWord(_SP);
	DecSP();
	reg16 = var;
}

void CPU::CALL(WORD addr)
{
	stackPush(_PC + 1);
	noInc = true;
	_PC = addr;
}


//Conditional call
void CPU::CALL(bool condition, WORD addr)
{
	if (condition)
	{
		clock += 12; //Have to add to clock, as branching takes longer than not branching
		stackPush(_PC + 1);
		noInc = true;
		_PC = addr;
	}
}
