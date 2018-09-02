#include "Window.h"



Window::Window()
{
	initialised = false;
}

HWND Window::GetHwnd()
{
	return windowHandle;
}
