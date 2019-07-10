#include "MansInterfacin.h"

#include <chrono>
#include <thread>



MansInterfacin::NativeWindowSystem::NativeWindowSystem(unsigned width, unsigned height, void(*callback_initedWindow)(NativeWindowSystem*)) 
	: width(width), height(height), callback_initedWindow(callback_initedWindow)
{}

HANDLE windowInitedEvent;
void MansInterfacin::NativeWindowSystem::InitWindow(LRESULT(CALLBACK* WindowProc_a)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)) {
	if (WindowProc_a)this->WindowProc_p = WindowProc_a;

	windowInitedEvent = CreateEventA(NULL, TRUE, FALSE, NULL);	//create the event that we're gonna wait for until the window is done initializing
	puts("creating thread to create window and init dx");
	this->windowThread = CreateThread(0, 0, (LPTHREAD_START_ROUTINE)MansInterfacin::NativeWindowSystem::windowInitializationAndRunningThreadFunc, (LPVOID)this, 0, 0);
	puts("thread created");
	WaitForSingleObject(windowInitedEvent, INFINITE);			//wait for the window to finish initializing and signal the event
	puts("shit inited");
	CloseHandle(windowInitedEvent);
}

void MansInterfacin::NativeWindowSystem::TermWindow() {
	PostMessageA(this->hWnd, WM_QUIT, 0, 0);					//let the message loop know we wanna shut this bitch down
	if (WaitForSingleObject(this->windowThread, 1000) == WAIT_TIMEOUT) { TerminateThread(this->windowThread, 0); puts("forcibly shutting down"); }
	// ^ wait for a second for the message loop to figure out it's done, if it doesn't then something fucked up, force the thread to exit
	this->windInited = false;
}

DWORD MansInterfacin::NativeWindowSystem::windowInitializationAndRunningThreadFunc(NativeWindowSystem* _this) {
	WNDCLASSA wc = { 0 };
	wc.lpfnWndProc = _this->WindowProc_p;
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpszClassName = "GaymeBoy";
	wc.hbrBackground = GetSysColorBrush(COLOR_BTNSHADOW);
	auto classerino = RegisterClassA(&wc);
	if (!classerino) {
		printf("Error registering class %d", GetLastError());
		return 0;
	}

	RECT desired_size = { 0, 0, (LONG)_this->width, (LONG)_this->height };
	AdjustWindowRect(&desired_size, WS_VISIBLE | WS_POPUP, FALSE);

	_this->hWnd = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST, (LPCSTR)classerino, "GaiBoi", WS_VISIBLE | WS_POPUP, 0, 0, desired_size.right - desired_size.left, desired_size.bottom - desired_size.top, 0, 0, 0, 0);
	if (!_this->hWnd) {
		printf("Error creating window %d", GetLastError());
		return 0;
	}

	_this->windInited = true;
	if (_this->callback_initedWindow)_this->callback_initedWindow(_this);

	puts("inited, signaling event");
	SetEvent(windowInitedEvent);							//signal the event to let the parent thread know we're done with initializing

	MSG msg;
	while (true) {
		if (PeekMessage(&msg, 0, 0, 0, 0)) {
			if (!GetMessage(&msg, 0, 0, 0))
				break; // got a WM_QUIT
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));		//TODO: 2 headers for this?
	}

	return 0;
}

void MansInterfacin::NativeWindowSystem::SetWindowProc(LRESULT(CALLBACK* WindowProc_a)(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)) {
	this->WindowProc_p = WindowProc_a;
	if (this->windInited)SetWindowLongPtr(this->hWnd, GWLP_WNDPROC, (LONG_PTR)WindowProc_a);
}

void MansInterfacin::NativeWindowSystem::SetWindowInitCallback(void(*callback_initedWindow_a)(NativeWindowSystem*)) {
	callback_initedWindow = callback_initedWindow_a;
}


LRESULT MansInterfacin::NativeWindowSystem::DefaultWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_DESTROY:
	{
		PostQuitMessage(0);
		return 0;
	}
	} return DefWindowProc(hWnd, message, wParam, lParam);
}


