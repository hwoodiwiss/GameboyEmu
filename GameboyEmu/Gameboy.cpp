#include "Gameboy.h"

std::shared_ptr<MMU> Gameboy::mmu;
bool Gameboy::running;

Gameboy::Gameboy()
{
	ROMLoaded = false;
	mmu = std::shared_ptr<MMU>(new MMU());
	cpu = std::unique_ptr<CPU>(new CPU(mmu));
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
#if(_DEBUG)
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

	gpu_thread->join();
	io_thread->join();
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
