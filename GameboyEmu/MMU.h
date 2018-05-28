#pragma once

#include "Shared.h"
#include "Bootstrap.h"

class MMU
{

public:
	MMU();
	~MMU();

	void LoadBootstrap();
	void LoadGraphic();

	WORD WriteByte(BYTE val);
	WORD WriteWord(WORD val);

	void WriteByte(WORD addr, BYTE val);
	void WriteWord(WORD addr, WORD val);

	BYTE ReadByte(WORD addr);
	WORD ReadWord(WORD addr);

private:
	BYTE memory[0xFFFF];
	WORD memAllocator;
	//Do more with this
	WORD memContext;
};