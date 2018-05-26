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
	CPU(MMU*);
	~CPU();

	void Boot();

private:
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

	//Memory manager
	MMU* _mmu;
};