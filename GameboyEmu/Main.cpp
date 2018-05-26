#include "MMU.h"
#include "CPU.h"
#include <stdio.h>

int main(void)
{
	MMU* memManager = new MMU();
	CPU zl80 = CPU(memManager);
	WORD byteAddr = memManager->WriteByte(32);

	printf("Byte val retrieved: %hu \n", memManager->ReadByte(byteAddr));

	WORD wordAddr = memManager->WriteWord(9999);
	memManager->WriteByte(32);
	printf("Word val retrieved: %hu\n", memManager->ReadWord(wordAddr));
	getchar();

	return 0;
}