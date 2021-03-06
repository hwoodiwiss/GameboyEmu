#include <Gameboy/Gameboy.h>

MMU* Gameboy::mmu;
bool Gameboy::running;

Gameboy::Gameboy()
{
	HINSTANCE instance = GetModuleHandle(NULL);
	ROMLoaded = false;
	mmu = new MMU();
	cpu = new CPU(mmu);

}

bool Gameboy::LoadROM(const char* fileName)
{
	return mmu->LoadROM(fileName);
}

bool Gameboy::LoadROM(char* fileName)
{
	return mmu->LoadROM(fileName);
}

void Gameboy::Run()
{
	cpu->Boot();
	running = true;
	//gpu_thread = std::unique_ptr<std::thread>(new std::thread(&Gameboy::GPUProcess));
	//io_thread = std::unique_ptr<std::thread>(new std::thread(&Gameboy::IOProcess));

	while (running)
	{
		try
		{
			cpu->Tick();
#if(_DEBUG && VERBOSE)
			cpu->DrawState();
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

	//gpu_thread->join();
	//io_thread->join();
}

void Gameboy::GPUProcess()
{
	while (running)
	{

	}
}

void Gameboy::IOProcess()
{
	while (running)
	{

	}
}
