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
	static std::shared_ptr<MMU> mmu;
	std::unique_ptr<CPU> cpu;
	std::unique_ptr<std::thread> io_thread;
	std::unique_ptr<std::thread> gpu_thread;
	static bool running;
};