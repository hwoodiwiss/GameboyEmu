#pragma once

#include "Shared.h"
#include "Bootstrap.h"


class MMU
{

public:
	MMU();

	void LoadBootstrap();

	bool LoadROM(const char* fileName);
	bool LoadROM(char* fileName);

	void WriteByte(WORD addr, BYTE val);
	void WriteWord(WORD addr, WORD val);

	void WriteWordBellow(WORD addr, WORD val);

	BYTE ReadByte(WORD addr);
	WORD ReadWord(WORD addr);

	WORD ReadWordBellow(WORD addr);

private:
	std::unique_ptr<BYTE[]> memory;

	BYTE romPage;
	bool bootromEnabled;
};