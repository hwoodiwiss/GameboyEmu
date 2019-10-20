#pragma once
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <vector>
#include <thread>
#include <mutex>

typedef void (*WndEvent)(WPARAM, LPARAM);

class Window
{
public:

	//If runThreaded is false, the message loop should be created and handled in the calling thread.
	Window(HINSTANCE appInstance, const char* WindowTitle, bool runThreaded);
	~Window();

	HWND GetHandle();

	void Add_OnLoad(WndEvent handler);
	void Remove_OnLoad(WndEvent handler);

	void Add_OnResize(WndEvent handler);
	void Remove_OnResize(WndEvent handler);

	void Add_OnExit(WndEvent handler);
	void Remove_OnExit(WndEvent handler);

	void Add_OnKeydown(WndEvent handler);
	void Remove_OnKeydown(WndEvent handler);

	bool is_running();

private:
	void ThreadProc(HINSTANCE);
	void MessageLoop();
	void set_running(bool value);

	LRESULT WndProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

	//Static window proc used to direct messages and params to the specific windows own Window proc
	static LRESULT GlobalMapWndProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam);

	//Events
	std::vector<WndEvent> OnLoad;
	std::vector<WndEvent> OnResize;
	std::vector<WndEvent> OnExit;
	std::vector<WndEvent> OnKeydown;

	//Internal usage
	ATOM m_pWndClass;
	HWND m_wndHandle;
	const char* WindowTitle;
	std::thread* m_messageLoop;
	bool m_running;
	std::mutex m_lockRunning;

};