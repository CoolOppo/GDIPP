#include "stdafx.h"
#include "gdipp_demo.h"
#include <gdipp_common.h>

using namespace std;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

  #define render
  #define test

int total_count = 5000;
vector<const wstring> candidate_font;
bool random_text = false;

DWORD start_time;
WCHAR window_title[100];

LONG CALLBACK create_minidump(__in struct _EXCEPTION_POINTERS *ExceptionInfo)
{
	MessageBox(NULL, L"A", L"B", MB_OK);
	return 0;
}

int APIENTRY wWinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPWSTR    lpCmdLine,
	int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_DEMO, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	HMODULE h_gdimm = NULL;

#ifdef render

#ifdef _M_X64
	h_gdimm = LoadLibrary(TEXT("gdimm_64.dll"));
#else
	h_gdimm = LoadLibrary(TEXT("gdimm_32.dll"));
#endif // _M_X64

#endif // render

#ifdef test
	wcs_convert(get_demo_setting("count"), total_count);
	candidate_font = get_demo_font();
	if (candidate_font.empty())
		candidate_font.push_back(L"Tahoma");
	wcs_convert(get_demo_setting("random_text"), random_text);

	window_title[0] = L'\0';

	//total_count = 0;
#endif // test

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DEMO));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	if (h_gdimm != NULL)
		FreeLibrary(h_gdimm);

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DEMO));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_DEMO);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

void show_result(HWND hWnd, HDC hdc)
{
	RedrawWindow(hWnd, NULL, NULL, RDW_ERASE | RDW_INVALIDATE | RDW_ERASENOW);

	SetTextColor(hdc, RGB(0, 0, 0));
	SetBkMode(hdc, TRANSPARENT);
	SetTextAlign(hdc, TA_LEFT | TA_TOP);

	const DWORD elapse_time = GetTickCount() - start_time;
	WCHAR result_str[100];
	swprintf(result_str, 100, L"%u milliseconds render time, %.2f ms per render", elapse_time, (double) elapse_time / total_count);

	HFONT f = CreateFontW(-20, 0, 0, 0, FW_REGULAR, 0, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, L"Tahoma");
	SelectObject(hdc, f);
	ExtTextOutW(hdc, 10, 10, 0, NULL, result_str, wcslen(result_str), NULL);
	DeleteObject(f);
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		{
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...

#ifdef test
		static int rendered = 0;

		if (window_title[0] == L'\0')
			GetWindowText(hWnd, window_title, 100);

		if (rendered == 0)
		{
			srand((unsigned) time(NULL));
			start_time = GetTickCount();
		}

		if (rendered <= total_count)
		{
			// randomize text color
			SetTextColor(hdc, RGB(rand() % 256, rand() % 256, rand() % 256));

			// randomize text background
			int bk_mode = rand() % 2 + 1;
			SetBkMode(hdc, bk_mode);
			if (bk_mode == OPAQUE)
				SetBkColor(hdc, RGB(rand() % 256, rand() % 256, rand() % 256));

			// randomize text position
			const int x = rand() % (ps.rcPaint.right - ps.rcPaint.left);
			const int y = rand() % (ps.rcPaint.bottom - ps.rcPaint.top);

			// randomize text metrics
			const LONG height = (rand() % 10) + 10;
			const LONG weight = (rand() % 9 + 1) * 100;
			const BYTE italic = rand() % 2;
			const wstring &font_name = candidate_font[rand() % candidate_font.size()];

			HFONT f = CreateFontW(-height, 0, 0, 0, weight, italic, 0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, font_name.c_str());
			SelectObject(hdc, f);

			// if randomize text content, use random Unicode characters
			// otherwise use the font name
			if (random_text)
			{
				const int max_text_len = 10;
				WCHAR render_str[max_text_len];
				const int render_len = rand() % max_text_len + 1;

				for (int i = 0; i < render_len; i++)
				{
					unsigned short chr;
					do 
					{
						chr = rand();
					} while (iswcntrl(chr));
					render_str[i] = chr;
				}

				ExtTextOutW(hdc, x, y, 0, NULL, render_str, render_len, NULL);
			}
			else
				ExtTextOutW(hdc, x, y, 0, NULL, font_name.c_str(), font_name.size(), NULL);

			DeleteObject(f);

			// show the rendered text count in the window title
			WCHAR new_title[100];
			wsprintf(new_title, TEXT("%s - %u"), window_title, rendered);
			SetWindowText(hWnd, new_title);

			// force redraw the client rect
			RedrawWindow(hWnd, NULL, NULL, RDW_INVALIDATE);
		}
		else if (rendered == total_count + 1)
			show_result(hWnd, hdc);

		rendered += 1;
#endif // test

		EndPaint(hWnd, &ps);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}
