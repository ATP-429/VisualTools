#define MAX(a, b, c) ((a > b) ? (a > c ? a : c) : (b > c ? b : c))
#define MIN(a, b, c) ((a < b) ? (a < c ? a : c) : (b < c ? b : c))

#include <vector>
#include <iostream>
#include <thread>

#include<chrono>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <gdiplus.h>
#include <windowsx.h>
#pragma comment(lib,"gdiplus.lib")

using namespace Gdiplus;
using namespace std;

// Global variables
int STARTX = GetSystemMetrics(SM_XVIRTUALSCREEN); //X coordinate of top-left of screen (May be 0 or -960 [1920/2] or something like that)
int STARTY = GetSystemMetrics(SM_YVIRTUALSCREEN); //Y coordinate of top-left of screen 
int WIDTH = GetSystemMetrics(SM_CXVIRTUALSCREEN); //Width of screen
int HEIGHT = GetSystemMetrics(SM_CYVIRTUALSCREEN); //Height of screen
HBITMAP screenShot, hiddenShot;

int X1 = -1, X2 = -1, Y1 = -1, Y2 = -1;

enum class ACTION {DEFAULT, SCREENSHOT, COLORPICK };

ACTION action = ACTION::DEFAULT;

// The main window class name.
static TCHAR szWindowClass[] = _T("DesktopApp");

// The string that appears in the application's title bar.
static TCHAR szTitle[] = _T("Snap Tool");

HINSTANCE hInst;

// Forward declarations of functions included in this code module:
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(
	_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPSTR     lpCmdLine,
	_In_ int       nCmdShow
)
{
	WNDCLASSEX wcex;
	if (strcmp(lpCmdLine, "ss") == 0)
	{
		action = ACTION::SCREENSHOT;
	}
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
	switch (action)
	{
		case ACTION::SCREENSHOT :
			wcex.hCursor = LoadCursor(NULL, IDC_CROSS);
			break;

		case ACTION::DEFAULT:
			wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	}
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL,
			_T("Call to RegisterClassEx failed!"),
			_T("Snap Tool"),
			NULL);

		return 1;
	}

	// Store instance handle in our global variable
	hInst = hInstance;

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;

	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	//Takes screenshot of screen and stores result in screenShot bitmap
	HDC screenDC = GetDC(NULL); //Gets DC for the whole screen
	HDC targetDC = CreateCompatibleDC(screenDC);
	screenShot = CreateCompatibleBitmap(screenDC, WIDTH, HEIGHT);
	SelectObject(targetDC, screenShot);
	BitBlt(targetDC, 0, 0, WIDTH, HEIGHT, screenDC, 0, 0, SRCCOPY);
	DeleteDC(targetDC); //we NEED to do this. If we don't do this, screenShot bmp is not available to display on the screen, since a bmp can only be held by one DC at a time

	//Stores the properly sized rectangle for us to have a client (display) area of size WIDTH x HEIGHT [Basically, gives us a window of WIDTH x HEIGHT excluding window borders]
	RECT properRect = { 0, 0, WIDTH, HEIGHT };
	AdjustWindowRectEx(&properRect, WS_OVERLAPPEDWINDOW, false, WS_EX_OVERLAPPEDWINDOW);

	HWND hWnd = CreateWindowEx(
		WS_EX_OVERLAPPEDWINDOW,
		szWindowClass,
		szTitle,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT,
		properRect.right - properRect.left, properRect.bottom - properRect.top, //Calculates size of required window from properRect
		NULL,
		NULL,
		hInstance,
		NULL
	);

	//Code taken from stackoverflow at https://stackoverflow.com/questions/2398746/removing-window-border
	//Gets rid of basic borders of the window
	LONG lStyle = GetWindowLong(hWnd, GWL_STYLE);
	lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
	SetWindowLong(hWnd, GWL_STYLE, lStyle);

	//Gets rid of extended borders (Removes that 1 black pixel border that offsets our window by 1 pixel) [Comment this part out and run to see what I mean]
	LONG lExStyle = GetWindowLong(hWnd, GWL_EXSTYLE);
	lExStyle &= ~(WS_EX_DLGMODALFRAME | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE);
	SetWindowLong(hWnd, GWL_EXSTYLE, lExStyle);

	SetWindowPos(hWnd, HWND_TOP, STARTX, STARTY, 0, 0, SWP_NOSIZE); //Sets window to top-left position without changing its size (SWP_NOSIZE)

	if (!hWnd)
	{
		MessageBox(NULL,
			_T("Call to CreateWindow failed!"),
			_T("Windows Desktop Guided Tour"),
			NULL);

		return 1;
	}

	ShowWindow(hWnd,
		nCmdShow);
	UpdateWindow(hWnd);

	// Main message loop:
	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	GdiplusShutdown(gdiplusToken);
	return (int)msg.wParam;
}

void render(HDC hdc)
{
	Graphics graphics(hdc);
	Pen pen(0xFFFF0000);
	SolidBrush darkBrush(0x44000000);
	SolidBrush lightBrush(0x88FFFFFF);

	int x1 = min(X1, X2);
	int x2 = max(X1, X2);
	int y1 = min(Y1, Y2);
	int y2 = max(Y1, Y2);

	graphics.DrawRectangle(&pen, x1, y1, x2 - x1 - 1, y2 - y1 - 1); //Draw red border around our selection
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc, bufferHDC, cropHDC;
	HBITMAP bufferBMP, cropBMP;
	RECT rect = { STARTX, STARTY, WIDTH, HEIGHT };
	TCHAR greeting[] = _T("Hello, Windows desktop!");
	int mouseX, mouseY;
	int x1, y1, x2, y2;

	switch (action)
	{
		case ACTION::SCREENSHOT:
			switch (message)
			{
				case WM_PAINT:
					hdc = BeginPaint(hWnd, &ps);
					bufferHDC = CreateCompatibleDC(hdc);

					//Copies screenShot to bufferHDC
					bufferBMP = (HBITMAP)CopyImage(screenShot, IMAGE_BITMAP, WIDTH, HEIGHT, LR_COPYFROMRESOURCE);
					SelectObject(bufferHDC, bufferBMP); //Sets bufferHDC to use bufferBMP

					render(bufferHDC); //Draws what we want onto bufferHDC, which is using bufferBMP rn

					BitBlt(hdc, 0, 0, WIDTH, HEIGHT, bufferHDC, STARTX, STARTY, SRCCOPY); //Copy result image from bufferHDC to hdc

					DeleteDC(bufferHDC); //we NEED to do this. If we don't do this, screenShot bmp is held by the previous bufferHDC, and thus the screenShot is not drawn again when our window is modified. You can see the effects of this by commenting out this line and re-sizing the window
					DeleteBitmap(bufferBMP); //We need to do this otherwise a lot of bitmaps get accumulated from repeated calling of InvalidateRect()
					EndPaint(hWnd, &ps);
					break;

				case WM_LBUTTONDOWN:
					mouseX = GET_X_LPARAM(lParam);
					mouseY = GET_Y_LPARAM(lParam);
					if (X1 == -1 && Y1 == -1)
					{
						X1 = mouseX;
						Y1 = mouseY;
					}
					else
					{
						X2 = mouseX;
						Y2 = mouseY;
					}
					break;

				case WM_MOUSEMOVE:
					mouseX = GET_X_LPARAM(lParam);
					mouseY = GET_Y_LPARAM(lParam);
					if (X1 != -1 && Y1 != -1)
					{
						X2 = mouseX;
						Y2 = mouseY;
					}
					InvalidateRect(hWnd, &rect, FALSE);
					break;

				case WM_LBUTTONUP:
					x1 = min(X1, X2);
					x2 = max(X1, X2);
					y1 = min(Y1, Y2);
					y2 = max(Y1, Y2);

					hdc = BeginPaint(hWnd, &ps);
					bufferHDC = CreateCompatibleDC(hdc); //Create new hdc
					cropHDC = CreateCompatibleDC(hdc); //Create new hdc that'll store croppedBMP

					bufferBMP = (HBITMAP)CopyImage(screenShot, IMAGE_BITMAP, WIDTH, HEIGHT, LR_COPYFROMRESOURCE); //Copies screenShot into bufferBMP
					cropBMP = CreateCompatibleBitmap(hdc, x2 - x1, y2 - y1); //Bitmap that will store our cropped screenshot

					SelectObject(bufferHDC, bufferBMP); //Sets bufferHDC to use bufferBMP
					SelectObject(cropHDC, cropBMP); //Sets hdc to use croppedBMP

					BitBlt(cropHDC, 0, 0, x2 - x1, y2 - y1, bufferHDC, x1, y1, SRCCOPY); //Copies our selection from bufferBMP to hdc, that is, into croppedBMP


					OpenClipboard(NULL);
					EmptyClipboard();
					SetClipboardData(CF_BITMAP, cropBMP);
					CloseClipboard();

					DeleteDC(bufferHDC);
					DeleteDC(cropHDC);

					EndPaint(hWnd, &ps);
					PostQuitMessage(0);
					break;

				case WM_KEYDOWN:
					if (wParam == VK_ESCAPE)
						PostQuitMessage(0);
					break;
				case WM_DESTROY:
					PostQuitMessage(0);
					break;

				default:
					return DefWindowProc(hWnd, message, wParam, lParam);
					break;
				case WM_ERASEBKGND:
					return 1;
			}
			break;

		case ACTION::COLORPICK:
			switch (message)
			{

			}
			break;
	}
	return 0;
}