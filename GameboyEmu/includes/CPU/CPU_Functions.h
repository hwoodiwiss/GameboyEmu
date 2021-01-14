#include <Shared.h>
#include <MMU/MMU.h>

//Flag definitions for the F register. To add a flag, OR it in.
extern BYTE f_Zero;
extern BYTE f_Subtract;
extern BYTE f_HalfCarry;
extern BYTE f_Carry;

inline void stackPush(MMU* mmu, WORD& stackPointer, WORD val)
{
	mmu->WriteWord(stackPointer, val);
	stackPointer += 2;
}

inline WORD stackPop(MMU* mmu, WORD& stackPointer)
{
	WORD val = mmu->ReadWord(stackPointer);
	stackPointer -= 2;
	return val;
}

inline void LD(BYTE& registerRef, BYTE operand)
{
	registerRef = operand;
}

inline void LD(WORD& wideRegRef, BYTE operand)
{
	wideRegRef = operand;
}

inline void LD(WORD& wideRegRef, WORD operand)
{
	wideRegRef = operand;
}

inline void LD(MMU&& mmu, WORD& _address, BYTE operand)
{
	mmu.WriteByte(_address, operand);
}

inline void ADD(BYTE& registerRef, BYTE& flagRegister, BYTE operand)
{
	WORD res = registerRef + operand;

	if ((((operand >> 8) & 0xF) + ((registerRef >> 8) & 0xF)) & 0x10)
	{
		flagRegister |= f_HalfCarry;
	}
	if (res & 0x100)
	{
		flagRegister |= f_Carry;
	}
	if (res == 0)
	{
		flagRegister |= f_Zero;
	}
	flagRegister &= ~f_Subtract;
	registerRef = (BYTE)res;
}

inline void ADD(WORD& registerRef, BYTE& flagRegister, WORD operand)
{
	WORD target = registerRef;

	WORD result = operand + target;

	registerRef = result;

	if ((((operand >> 8) & 0xF) + ((target >> 8) & 0xF)) & 0x10)
	{
		flagRegister |= f_HalfCarry;
	}
	if ((operand & 0x80) && (target & 0x80))
	{
		flagRegister |= f_Carry;
	}
	flagRegister &= ~f_Subtract;
}

inline void ADC(BYTE& registerRef, BYTE& flagRegister, BYTE operand)
{
	if ((registerRef & 0x8) == 0x8)
	{
		flagRegister |= f_HalfCarry;
	}
	registerRef = registerRef + operand;
	if (registerRef == 0)
	{
		flagRegister |= f_Zero;
	}
}

inline void ADC(BYTE& registerRef, WORD operand)
{
}

inline void BIT(BYTE& registerRef, BYTE& flagRegister, BYTE nBit)
{
	BYTE nMask = ((BYTE)1 << (BYTE)nBit);
	if ((registerRef & nMask) == nMask)
	{
		registerRef &= ~nMask;
	}
	else
	{
		flagRegister |= nMask;
	}
	flagRegister &= ~f_Subtract;
	flagRegister |= f_HalfCarry;
}

inline void SUB(BYTE operand)
{
}

inline void SUB(WORD operand)
{
}

inline void SBC(BYTE& registerRef, BYTE& flagRegister, BYTE operand)
{
	if ((registerRef & 0x10) == 0x10)
	{
		flagRegister |= f_HalfCarry;
	}
	registerRef -= operand;
	if (registerRef == 0)
	{
		flagRegister |= f_Zero;
	}
	flagRegister |= f_Subtract;
}

inline void SBC(BYTE& registerRef, WORD operand)
{
}

inline void INC(BYTE& registerRef, BYTE& flagRegister)
{
	if ((registerRef & 0xF) == 0xF)
	{
		flagRegister |= f_HalfCarry;
	}
	registerRef++;
	if (registerRef == 0)
	{
		flagRegister |= f_Zero;
	}
	flagRegister &= ~f_Subtract;
}

//Increment a 16-bit register value.
//TODO: Add flag setting
inline void INC(WORD& reg16, BYTE& flagRegister)
{
	reg16++;
}

//8-bit decrement. Extra complex, as we have to decrement the value, and set all the required flags.
inline void DEC(BYTE& registerRef, BYTE& flagRegister)
{
	if ((registerRef & 0x10) == 0x10)
	{
		flagRegister |= f_HalfCarry;
	}
	registerRef--;
	if (registerRef == 0)
	{
		flagRegister |= f_Zero;
	}
	flagRegister |= f_Subtract;
}

inline void DEC(WORD& reg16)
{
	reg16--;
}

inline void JR(unsigned long long& clock, WORD& programCounter, bool condition, SBYTE offset)
{
	if (condition)
	{
		clock += 4; //Have to add to clock, as branching takes longer than not branching
		programCounter += offset;
	}
}

inline void XOR(BYTE& registerRef, BYTE& flagRegister, BYTE operand)
{
	registerRef ^= operand;
	if (registerRef == 0)
	{
		flagRegister |= f_Zero;
	}
	flagRegister &= ~f_Carry;
	flagRegister &= ~f_HalfCarry;
	flagRegister &= ~f_Subtract;
}

//Unconditional return
inline void RET(MMU* mmu, WORD& stackPointer, WORD& programCounter)
{
	programCounter = stackPop(mmu, stackPointer);
}

//Conditional return
inline void RET(MMU* mmu, WORD& stackPointer, WORD& programCounter, unsigned long long& clock, bool condition)
{
	if (condition)
	{
		clock += 12; //Have to add to clock, as branching takes longer than not branching
		programCounter = stackPop(mmu, stackPointer);
	}
}

inline void PUSH(MMU* mmu, WORD& stackPointer, WORD address)
{
	stackPush(mmu, stackPointer, address);
}

inline void POP(MMU* mmu, WORD& stackPointer, WORD& reg16)
{
	reg16 = stackPop(mmu, stackPointer);
}

inline void CALL(MMU* mmu, WORD& stackPointer, WORD& programCounter, bool& noInc, WORD addr)
{
	stackPush(mmu, stackPointer, programCounter + 1);
	noInc = true;
	programCounter = addr;
}


//Conditional call
inline void CALL(MMU* mmu, WORD& stackPointer, WORD& programCounter, unsigned long long& clock, bool& noInc, bool condition, WORD addr)
{
	if (condition)
	{
		clock += 12; //Have to add to clock, as branching takes longer than not branching
		stackPush(mmu, stackPointer, programCounter + 1);
		noInc = true;
		programCounter = addr;
	}
}

