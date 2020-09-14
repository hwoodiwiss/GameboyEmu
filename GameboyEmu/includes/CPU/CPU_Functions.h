#include <Shared.h>
#include <MMU/MMU.h>

BYTE f_Zero = 0x80;
BYTE f_Subtract = 0x40;
BYTE f_HalfCarry = 0x20;
BYTE f_Carry = 0x10;

inline void LD(BYTE& _register, BYTE& operand)
{
	_register = operand;
}

inline void LD(WORD& _reg16, WORD& operand)
{
	_reg16 = operand;
}

inline void LD(MMU&& mmu, WORD& _address, BYTE& operand)
{
	mmu.WriteByte(_address, operand);
}

inline void ADD(BYTE& _register, BYTE& flagRegister, BYTE& operand)
{
	WORD res = _register + operand;

	if ((((operand >> 8) & 0xF) + ((_register >> 8) & 0xF)) & 0x10)
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
	_register = (BYTE)res;
}

inline void ADD(WORD& _register, BYTE& flagRegister, WORD operand)
{
	WORD target = _register;

	WORD result = operand + target;

	_register = result;

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

inline void ADC(BYTE& _register, BYTE& flagRegister, BYTE& operand)
{
	if ((_register & 0x8) == 0x8)
	{
		flagRegister |= f_HalfCarry;
	}
	_register = _register + operand;
	if (_register == 0)
	{
		flagRegister |= f_Zero;
	}
}

inline void ADC(BYTE& _register, WORD operand)
{
}

inline void BIT(BYTE& _register, BYTE nBit)
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

inline void SUB(BYTE operand)
{
}

inline void SUB(WORD operand)
{
}

inline void SBC(BYTE& _register, BYTE operand)
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

inline void SBC(BYTE& _register, WORD operand)
{
}

inline void INC(BYTE& _register)
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
inline void INC(WORD& reg16)
{
	reg16++;
}

//8-bit decrement. Extra complex, as we have to decrement the value, and set all the required flags.
inline void DEC(BYTE& _register)
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

inline void DEC(WORD& reg16)
{
	reg16--;
}

inline void JR(bool condition, SBYTE offset)
{
	if (condition)
	{
		clock += 4; //Have to add to clock, as branching takes longer than not branching
		_PC += offset;
	}
}

inline void XOR(BYTE operand)
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
inline void RET()
{
	_PC = stackPop();
}

//Conditional return
inline void RET(bool condition)
{
	if (condition)
	{
		clock += 12; //Have to add to clock, as branching takes longer than not branching
		_PC = stackPop();
	}
}

inline void PUSH(WORD address)
{
	stackPush(address);
}

inline void POP(WORD& reg16)
{
	WORD var = _mmu->ReadWord(_SP);
	DecSP();
	reg16 = var;
}

inline void CALL(WORD addr)
{
	stackPush(_PC + 1);
	noInc = true;
	_PC = addr;
}


//Conditional call
inline void CALL(bool condition, WORD addr)
{
	if (condition)
	{
		clock += 12; //Have to add to clock, as branching takes longer than not branching
		stackPush(_PC + 1);
		noInc = true;
		_PC = addr;
	}