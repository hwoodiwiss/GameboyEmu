#include "Instruction.h"

Instruction::Instruction(const char * name, Op function)
{
	_name = name;
	_operation = function;
}

Op Instruction::GetOperation()
{
	return _operation;
}
