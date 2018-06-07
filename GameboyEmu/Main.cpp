#include "MMU.h"
#include "CPU.h"
#include <stdio.h>

int main(void)
{
	MMU* mmu = new MMU();
	CPU cpu = CPU(mmu);
	cpu.Boot();
	bool running = true;
	while (running)
	{
		try
		{
			cpu.Tick();
#if(_DEBUG)
			//cpu.DrawState();
#endif
		}
		catch (WORD opCodeAndPC)
		{
			BYTE opCode;
			BYTE PC;
			WordToBytes(opCode, PC, opCodeAndPC);
			printf("Operation not implemented!\nTalk to Hugo.\nOpcode: %02X\nProgram Counter: H: %02X D: %d\n", opCode, PC, PC);
			running = false;
		}
	}

	printf("Press any key to exit...");
	getchar();

	return 0;
}