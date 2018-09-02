#pragma once

#include <Windows.h>
#define WIN32_LEAN_AND_MEAN

class Window
{
public:
	Window();
	HWND GetHwnd();

private:
	bool initialised;
	HWND windowHandle;

};