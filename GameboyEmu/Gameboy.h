#pragma once

#include "MMU.h"
#include "CPU.h"
#include "Shared.h"
#include <iostream>
#include <fstream>

static class Gameboy 
{
public:
	Gameboy();
	bool LoadROM(const char* fileName);
	bool LoadROM(char* fileName);
	void Run();

private:
	static void GPUProcess();
	static void IOProcess();


	bool ROMLoaded;
	static MMU* mmu;
	CPU* cpu;
	Window* window;
	std::thread* io_thread;
	std::thread* gpu_thread;
	static bool running;
};