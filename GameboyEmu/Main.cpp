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
		}
		catch (const char* ex)
		{
			printf(ex);
			running = false;
		}
	}

	printf("Press any key to exit...");
	getchar();

	return 0;
}