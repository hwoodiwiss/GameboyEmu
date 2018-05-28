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

	void JR_NZ_0x20();
	void LD_HL_0x21();
	void LD_SP_0x31();
	void LD_HL_MINUS_0x32();
	void LD_HL_H_0x66();
	void SBC_A_A_0x9F();
	void XOR_A_0xAF();
	void PREFIX_0xCB();
	void CALL_Z_0xCC();
	void ADC_A_0xCE();
	void BIT_H_7_0xCB7C();

	CPU(MMU*);
	void RegInstruction(WORD opCode, BYTE duration, InstructionFunc function);
	~CPU();

	void Boot();

	void Tick();



	WORD GetSP();

	void stackPush(WORD val);
	WORD stackPop();
private:
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


	Instruction instructions[0xFF];
	Instruction cbPageInstructions[0xFF];

	//Memory manager
	MMU* _mmu;

	//Instructions
	//OpCode operations[0xFF];
};