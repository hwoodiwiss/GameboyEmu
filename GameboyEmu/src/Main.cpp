#include <Gameboy/Gameboy.h>
#include <Shared.h>

int main(void)
{
	std::unique_ptr<Gameboy> gameboy(new Gameboy());
	if (gameboy->LoadROM("Game.bin"))
	{
		gameboy->Run();
	}

	printf("Press any key to exit...");
	getchar();

	return 0;
}