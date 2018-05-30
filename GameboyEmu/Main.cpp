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
			cpu.DrawState();
#endif
		}
		catch (BYTE opCode)
		{
			printf("Operation not implemented!\nTalk to Hugo.\n %X\n", opCode);
			running = false;
		}
	}

	printf("Press any key to exit...");
	getchar();

	return 0;
}