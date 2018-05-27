#pragma once

#include "Shared.h"

class Instruction
{
public:
	Instruction(const char* name, Op function);
	Op GetOperation();
private:
	const char* _name;
	Op _operation;
};