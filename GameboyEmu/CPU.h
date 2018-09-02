#pragma once

#include "MMU.h"
#include "Shared.h"

//Flag definitions for the F register. To add a flag, OR it in.
extern BYTE f_Zero;
extern BYTE f_Subtract;
extern BYTE f_HalfCarry;
extern BYTE f_Carry;

class CPU
{
public:
	typedef void (CPU::*InstructionFunc)();

	struct Instruction
	{
		BYTE opCode;
		BYTE duration;
		InstructionFunc function;
	};

	CPU(std::shared_ptr<MMU>);
	void RegInstruction(WORD opCode, InstructionFunc function);
	~CPU();

	void Boot();

	void Tick();

#if(_DEBUG)
	void DrawState();
#endif

private:

	WORD GetSP();

	void stackPush(WORD val);
	WORD stackPop();
	void IncSP();
	void DecSP();

	//Registers
	
	/*
		Registers arranged:
		A | F
		B | C
		D | E
		H | L
		  SP
		  PC
		
		A is the Accumalator
		B, C, D, E, H and L are general purpose registers.
		SP is the stack pointer
		PC is the Program Counter
		Any 2 neighbouring 8-bit registers can be used to store a 16 bit value
	*/
	BYTE A, F, B, C, D, E, H, L;

	WORD _SP, _PC;

	unsigned long long clock;

	BYTE IF;
	bool IME;
	bool HALT;

#if(_DEBUG)
	WORD prevOp;
#endif

	Instruction instructions[0xFF];
	Instruction cbPageInstructions[0xFF];

	//Memory manager
	std::shared_ptr<MMU> _mmu;
	
	//Instructions

	void LD(BYTE* _register, BYTE operand);
	void LD(BYTE* regLow, BYTE* regHigh, WORD operand);
	void LD(WORD _address, BYTE operand);
	void ADD(BYTE operand); //Result always in A
	void ADD(WORD operand); // Result always in HL
	void ADC(BYTE* _register, BYTE* operand);
	void ADC(BYTE* _register, BYTE operand);
	void ADC(BYTE* _register, WORD operand);
	void SUB(BYTE operand);
	void SUB(WORD operand);
	void SBC(BYTE* _register, BYTE operand);
	void SBC(BYTE* _register, WORD operand);
	void INC(BYTE* _register);
	void INC(BYTE* regLow, BYTE* regHigh);
	void DEC(BYTE* _register);
	void DEC(BYTE* regLow, BYTE* regHigh);
	void BIT(BYTE* _register, BYTE bit);
	void JR(bool condition, SBYTE address);
	void XOR(BYTE operand);
	void RET();
	void RET(bool condition);
	void PUSH(WORD address);
	void POP(BYTE* regLow, BYTE* regHigh);
	void CALL(bool condition, WORD addr);

	//OpCode operations[0xFF];
	bool noInc;

	void NOP_0x00();
	void LD_BC_d16_0x01();
	void LD_pBC_A_0x02();
	void INC_BC_0x03();
	void INC_B_0x04();
	void DEC_B_0x05();
	void LD_B_d8_0x06();
	void RLCA_0x07();
	void LD_pd16_SP_0x08();
	void ADD_HL_BC_0x09();
	void LD_A_pBC_0x0A();
	void DEC_BC_0x0B();
	void INC_C_0x0C();
	void DEC_C_0x0D();
	void LD_C_d8_0x0E();
	void RRCA_0x0F();
	void STOP_0x10();
	void LD_DE_d16_0x11();
	void LD_pDE_A_0x12();
	void INC_DE_0x13();
	void RLA_0x17();
	void AND_C_0x1A();
	void LD_E_d8_0x1E();
	void JR_NZ_0x20();
	void LD_HL_0x21();
	void LD_HL_PLUS_0x22();
	void INC_HL_0x23();
	void JR_Z_r8_0x28();
	void LD_SP_0x31();
	void LD_HL_MINUS_0x32();
	void INC_A_0x3C();
	void DEC_A_0x3D();
	void LD_A_d8_0x3E();
	void LD_C_A_0x4F();	
	void LD_D_A_0x57();
	void LD_HL_H_0x66();
	void LD_H_A_0x67();
	void LD_pHL_A_0x77();
	void LD_A_E_0x7B();
	void ADD_A_B_0x80();
	void SBC_A_A_0x9F();
	void XOR_A_0xAF();
	void RET_NZ_0xC1();
	void PUSH_BC_0xC5();
	void PREFIX_0xCB();
	void CALL_Z_0xCC();
	void CALL_d16_0xCD();
	void ADC_A_0xCE();
	void POP_HL_0xE1();
	void LD_pC_A_0xE2();
	void LD_pa16_A_0xEA();
	void LDH_pa8_A_0xE0();
	void LDH_0xF0();
	void DI_0xF3();
	void CP_d8_0xFE();
	void BIT_H_7_0xCB7C();
	void RL_C_0xCB11();
};