#include "MMU.h"
#include "CPU.h"
#include <stdio.h>

int main(void)
{
	MMU* mmu = new MMU();
	CPU cpu = CPU(mmu);
	WORD byteAddr = mmu->WriteByte(32);
	cpu.Boot();
	while (true)
	{
		cpu.Tick();
	}

	return 0;
}