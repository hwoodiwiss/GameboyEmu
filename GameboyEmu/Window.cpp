#include "Window.h"
#include <algorithm>

Window::Window(HINSTANCE appInstance, const char* WindowTitle, bool runThreaded) : WindowTitle(WindowTitle)
{
	WNDCLASSEX winDesc;
	winDesc.cbSize = sizeof(WNDCLASSEX);
	winDesc.style = 0;
	winDesc.cbClsExtra = 0;
	winDesc.cbWndExtra = 0;
	winDesc.hInstance = appInstance;
	winDesc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	winDesc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	winDesc.hCursor = LoadCursor(NULL, IDC_ARROW);
	winDesc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	winDesc.lpfnWndProc = &GlobalMapWndProc;
	winDesc.lpszMenuName = NULL;
	winDesc.lpszClassName = "WindowClass";

	m_pWndClass = RegisterClassEx(&winDesc);

	if (m_pWndClass == NULL)
	{
		throw GetLastError();
	}

	DWORD winStyle = 0;
	DWORD winStyleEx = WS_EX_APPWINDOW;

	if (!runThreaded)
	{

		//Creates window handle, passing the "this" pointer as creation LPARAM
		m_wndHandle = CreateWindowEx(WS_EX_CLIENTEDGE, "WindowClass", WindowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 500, NULL, NULL, appInstance, this);

		if (m_wndHandle == NULL)
		{
			throw GetLastError();
		}

		ShowWindow(m_wndHandle, SW_SHOW);

		set_running(true);

	}
	else
	{
		set_running(true);

		m_messageLoop = new std::thread(&Window::ThreadProc, this, appInstance);
	}
}

Window::~Window()
{
	if (m_messageLoop != NULL)
		m_messageLoop->join();
}

void Window::ThreadProc(HINSTANCE exInstance)
{
	m_wndHandle = CreateWindowEx(WS_EX_CLIENTEDGE, "WindowClass", WindowTitle, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 500, NULL, NULL, exInstance, this);

	if (m_wndHandle == NULL)
	{
		throw GetLastError();
	}

	ShowWindow(m_wndHandle, SW_SHOW);

	MessageLoop();
}

HWND Window::GetHandle()
{
	if (m_wndHandle == NULL)
		return NULL;

	return m_wndHandle;
}

void Window::Add_OnLoad(WndEvent handler)
{
	if (std::find(OnLoad.begin(), OnLoad.end(), handler) == OnLoad.end())
	{
		OnLoad.push_back(handler);
	}
}

void Window::Remove_OnLoad(WndEvent handler)
{
	for (auto it = OnLoad.begin(); it != OnLoad.end(); it++)
	{
		if ((*it) == handler)
		{
			OnLoad.erase(it);
		}
	}
}

void Window::Add_OnResize(WndEvent handler)
{
	if (std::find(OnResize.begin(), OnResize.end(), handler) == OnResize.end())
	{
		OnResize.push_back(handler);
	}
}

void Window::Remove_OnResize(WndEvent handler)
{
	for (auto it = OnResize.begin(); it != OnResize.end(); it++)
	{
		if ((*it) == handler)
		{
			OnResize.erase(it);
		}
	}
}

void Window::Add_OnExit(WndEvent handler)
{
	if (std::find(OnExit.begin(), OnExit.end(), handler) == OnExit.end())
	{
		OnExit.push_back(handler);
	}
}

void Window::Remove_OnExit(WndEvent handler)
{
	for (auto it = OnExit.begin(); it != OnExit.end(); it++)
	{
		if ((*it) == handler)
		{
			OnExit.erase(it);
		}
	}
}

void Window::Add_OnKeydown(WndEvent handler)
{
	if (std::find(OnKeydown.begin(), OnKeydown.end(), handler) == OnKeydown.end())
	{
		OnKeydown.push_back(handler);
	}
}

void Window::Remove_OnKeydown(WndEvent handler)
{
	for (auto it = OnKeydown.begin(); it != OnKeydown.end(); it++)
	{
		if ((*it) == handler)
		{
			OnKeydown.erase(it);
		}
	}
}

bool Window::is_running()
{
	bool ret;
	if (!m_lockRunning.try_lock())
		return true;
	ret = m_running;
	m_lockRunning.unlock();
	return ret;
}

void Window::set_running(bool value)
{
	m_lockRunning.lock();
	m_running = value;
	m_lockRunning.unlock();
}

void Window::MessageLoop()
{
	MSG message;
	BOOL bRet;

	while (is_running())
	{
		while (PeekMessage(&message, m_wndHandle, NULL, NULL, PM_REMOVE))
		{

			TranslateMessage(&message);
			DispatchMessage(&message);
		}
	}

	set_running(false);
}

LRESULT Window::WndProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		for (auto handler : OnLoad)
		{
			handler(wParam, lParam);
		}
		break;
	case WM_SETREDRAW:
		for (auto handler : OnResize)
		{
			handler(wParam, lParam);
		}
		break;
	case WM_KEYDOWN:
		for (auto handler : OnKeydown)
		{
			handler(wParam, lParam);
		}
		break;
	case WM_CLOSE:
		for (auto handler : OnExit)
		{
			handler(wParam, lParam);
		}
		set_running(false);
		break;
	default:
		break;
	}

	return DefWindowProc(windowHandle, message, wParam, lParam);
}

LRESULT Window::GlobalMapWndProc(HWND windowHandle, UINT message, WPARAM wParam, LPARAM lParam)
{
	Window* pThis;

	if (message == WM_NCCREATE)
	{
		//Gets the "this" pointer passed to CreateWindowEx
		pThis = static_cast<Window*>(reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams);

		SetLastErrorEx(0, 0);

		//Store a pointer to the window object in the window GWLP_USERDATA
		if (!SetWindowLongPtr(windowHandle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis)))
		{
			if (GetLastError() != 0)
				return FALSE;
		}
	}
	else
	{
		//Get the pointer to the window object from the Windows window
		pThis = reinterpret_cast<Window*>(GetWindowLongPtr(windowHandle, GWLP_USERDATA));
	}

	//Return the WndProc for the Window object
	return pThis->WndProc(windowHandle, message, wParam, lParam);
}