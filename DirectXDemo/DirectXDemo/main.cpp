#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "BasicDemo.h"
#include <memory>
#include "AntTweakBar.h"

LRESULT CALLBACK wndProc(HWND _hwnd, UINT _msg, WPARAM _wParam, LPARAM _lParam);

int WINAPI wWinMain(HINSTANCE _hInstance, HINSTANCE _prevInstance, LPWSTR _cmdLine, int _cmdShow)
{
	UNREFERENCED_PARAMETER(_prevInstance);
	UNREFERENCED_PARAMETER(_cmdLine);

	WNDCLASSEX wndClass = { 0 };
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = wndProc;
	wndClass.hInstance = _hInstance;
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = "DX11WinClass";

	if (!RegisterClassEx(&wndClass)) return -1;

	//RECT rc = { 0,0,640,480 };
	RECT rc = { 0,0, 1000, 600 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);

	HWND hwnd = CreateWindowA("DX11WinClass", "DX11 Demo", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
		rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, _hInstance, NULL);

	if (!hwnd) return -1;

	ShowWindow(hwnd, _cmdShow);

	//init demo
	std::unique_ptr<DX11Demo> demo(new BasicDemo());
	bool result = demo->init(_hInstance, hwnd);
	if (result == false) return -1;

	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			demo->update();
			demo->render();
		}
	}

	demo->shutdown();
	return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK wndProc(HWND _hwnd, UINT _msg, WPARAM _wParam, LPARAM _lParam)
{
	if (TwEventWin(_hwnd, _msg, _wParam, _lParam))
		return 0; // Event has been handled by AntTweakBar

	PAINTSTRUCT paintStruct;
	HDC hdc;

	switch (_msg)
	{
	case WM_PAINT:
		hdc = BeginPaint(_hwnd, &paintStruct);
		EndPaint(_hwnd, &paintStruct);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(_hwnd, _msg, _wParam, _lParam);
	}

	return 0;
}
