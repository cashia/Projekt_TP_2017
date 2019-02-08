// Win32Project1.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Win32Project1.h"
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>

#define MAX_LOADSTRING 100
#define TMR_1 1

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

struct point {
	double x, y, z;
	point(double x, double y, double z) : x(x), y(y), z(z) {};
};
struct speed {
	double v_x, v_y, v_z;
	speed(double v_x, double v_y, double v_z) : v_x(v_x), v_y(v_y), v_z(v_z) {};
};

INT amplituda=70;
INT podzialka_czasu=10;
INT dalej = 0;

HWND hwndButton;

// sent data
int col = 0;
std::vector<point> points; // wektor punktow odczytanych z pliku
std::vector<speed> speeds; //wektor wspolrzednych predkosci
std::vector<double> _przyspieszenie; //wyliczone wartosci przyspieszenia
std::vector<double> _predkosc; //wyliczone wartosci predkosci
std::vector<double> _droga;  //wyliczone chwilowe drogi
std::vector<double> _distance; //kolejne sumy chwilowych drog
RECT drawArea1 = { 200, 0, 1000, 500 };
RECT drawArea2 = { 190, 500, 1000, 550 };


void MyOnPaint(HDC hdc, BOOL id1, BOOL id2, BOOL id3, HWND hWnd) //tu trzeba dopasowac rysowanie wykresu
{
	Graphics graphics(hdc);
	Pen pen(Color(255, 255, 0, 0));
	Pen pen1(Color(255, 0, 128, 0));
	Pen pen2(Color(255, 0, 0, 255));

	int liczba = _przyspieszenie.size(); //liczba pomiarow
	float pom = 1.0;
	bool komunikat = false;
	if (id1)
	{
		std::vector<double>::iterator it;
		if (liczba > dalej)
			for (it = _przyspieszenie.begin()+dalej; it != _przyspieszenie.end()-1; )
			{
				double x = *it;
				double y = *(++it);
				double q = x * amplituda;
				double w = y * amplituda;
				graphics.DrawLine(&pen, (pom - 1.0)*podzialka_czasu, 500.0 - q, pom*podzialka_czasu, 500.0 - w); // bo lewy dolny punkt pola ma wsp (0, 200) wzgledem drawArea1, a ma przedstawiac (0,0)
				pom++;
				komunikat = false;
			}
		else if(!komunikat)
		{
			MessageBox(hWnd, TEXT("Przekroczono liczbê pomiarów!"), NULL, MB_OK);
			komunikat = true;
		}
		pom = 1.0;
	}
	if (id2)
	{
		std::vector<double>::iterator it;
		if (liczba > dalej)
			for (it = _predkosc.begin()+dalej; it != _predkosc.end()-1; )
			{
				double x = *it;
				double y = *(++it);
				double q = x * amplituda;
				double w = y * amplituda;
				graphics.DrawLine(&pen1, (pom - 1.0)*podzialka_czasu, 500 - q, pom*podzialka_czasu, 500 - w); // bo lewy dolny punkt pola ma wsp (0, 200) wzgledem drawArea1, a ma przedstawiac (0,0)
				pom++;
				komunikat = false;
			}
		else if(!komunikat)
		{
			MessageBox(hWnd, TEXT("Przekroczono liczbê pomiarów!"), NULL, MB_OK);
			komunikat = true;
		}
		pom = 1.0;
	}
	if (id3)
	{
		std::vector<double>::iterator it;
		if (liczba > dalej)
			for (it = _distance.begin()+dalej; it != _distance.end()-1; )
			{
				double x = *it;
				double y = *(++it);
				double q = x * amplituda;
				double w = y * amplituda;
				graphics.DrawLine(&pen2, (pom - 1.0)*podzialka_czasu, 500 - q, pom*podzialka_czasu, 500 - w); // bo lewy dolny punkt pola ma wsp (0, 200) wzgledem drawArea1, a ma przedstawiac (0,0)
				pom++;
				komunikat = false;
			}
		else if(!komunikat)
		{
			MessageBox(hWnd, TEXT("Przekroczono liczbê pomiarów!"), NULL, MB_OK);
			komunikat = true;
		}
		pom = 1.0;
	}
}


void rysuj_uklad(HDC hdc)
{
	Graphics graphics(hdc);
	Pen pen(Color(255, 0, 0, 0));

	graphics.DrawLine(&pen, 190, 510, 1000, 510); //Rysowanie linii osi

	TextOutW(hdc, 970, 520, L"T[s]", 4); //Podpisanie osi

	int liczba = _przyspieszenie.size(); //liczba pomiarow

	for (int i = 1; i <= liczba; i++) 
	{
		int sekunda = 25 * podzialka_czasu;
		if (i % sekunda == 0)
			graphics.DrawLine(&pen, 200 + i, 508, 200 + i, 512);
	}
}

void repaintWindow(HWND hWnd, HDC &hdc, PAINTSTRUCT &ps, RECT *drawArea1, RECT *drawArea2, BOOL id1, BOOL id2, BOOL id3)
{
	InvalidateRect(hWnd, drawArea1, TRUE);
	hdc = BeginPaint(hWnd, &ps);
	MyOnPaint(hdc, id1, id2, id3, hWnd);
	EndPaint(hWnd, &ps);
	InvalidateRect(hWnd, drawArea2, TRUE);
	hdc = BeginPaint(hWnd, &ps);
	rysuj_uklad(hdc);
	EndPaint(hWnd, &ps);
}

void liczPrzyspieszenie()
{
	std::vector<point>::iterator it;
	for (it = points.begin(); it != points.end(); it++)
	{
		double x = it->x;
		double y = it->y;
		double z = it->z;

		double przyspieszenie = sqrt(x*x + y*y + z*z);
		_przyspieszenie.push_back(przyspieszenie);
	}
}

void liczPredkosc()
{
	std::vector<point>::iterator it;
	for (it = points.begin(); it != points.end(); it++)
	{
		double x = it->x;
		double y = it->y;
		double z = it->z;
		double pom1 = 0.04; //granice calkowania (t2-t1)=0.04

		double v_x = x * pom1; //skladowe predkosci
		double v_y = y * pom1;
		double v_z = z * pom1;
		speeds.push_back(speed(v_x, v_y, v_z));
	}
}

void liczDroge()
{
	std::vector<speed>::iterator it;
	for (it = speeds.begin(); it != speeds.end(); it++)
	{
		double x = it->v_x;
		double y = it->v_y;
		double z = it->v_z;
		double pom1 = 0.04;

		double s_x = x * pom1; //skladowe wektora przemieszczenia
		double s_y = y * pom1;
		double s_z = z * pom1;
		double droga = sqrt(s_x*s_x + s_y*s_y + s_z*s_z);
		_droga.push_back(droga);
	}
}

void countSpeed()
{
	double pom1 = 0;
	double pom2 = 0;
	double pom3 = 0;
	std::vector<speed>::iterator it;
	for (it = speeds.begin(); it != speeds.end(); it++)
	{
		double x = it->v_x;
		double y = it->v_y;
		double z = it->v_z;

		double v_x = x + pom1;
		double v_y = y + pom2;
		double v_z = z + pom3;

		pom1 = v_x;
		pom2 = v_y;
		pom3 = v_z;

		double predkosc = sqrt(v_x*v_x + v_y*v_y + v_z*v_z);// wartosc predkosci
		_predkosc.push_back(predkosc);
	}
}

void countDistance()
{
	double pom = 0;
	std::vector<double>::iterator it;
	for (it = _droga.begin(); it != _droga.end(); it++)
	{
		double droga = *it;

		double distance = droga + pom;

		pom = distance;

		_distance.push_back(distance);
	}
}

void inputData()
{
	std::ifstream plik("outputRobotForwardA01.log");
	if (plik.good() == true)
	{
		double pom1, pom2, pom3;
		int i = 1;
		while (plik >> pom1 >> pom2 >> pom3)
		{
			if (i==2)
			{
				double pom4 = pom3 + 1;

				//zmieniamy jednostki przyspieszenia z 1G na 1m/s^2
				double a_x = pom1 * 9.81;
				double a_y = pom2 * 9.81;
				double a_z = pom4 * 9.81;
				points.push_back(point(a_x, a_y, a_z));
				i++;
				continue;
			}
 			else if (i == 4)
			{
				i = 1;
				continue;
			}
			i++;
		}
	}
	plik.close();
	liczPrzyspieszenie();
	liczPredkosc();
	liczDroge();
	countSpeed();
	countDistance();
}

int OnCreate(HWND window)
{
	inputData();
	return 0;
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.
	MSG msg;

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR           gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_WIN32PROJECT1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32PROJECT1));

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

	GdiplusShutdown(gdiplusToken);
    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32PROJECT1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WIN32PROJECT1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
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
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   //Przyciski checkbox

   hwndButton = CreateWindow(TEXT("button"), TEXT("Przyspieszenie"),
	   WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
	   0, 0, 150, 30, hWnd, (HMENU)ID_RBUTTON1, GetModuleHandle(NULL), NULL);
   hwndButton = CreateWindow(TEXT("button"), TEXT("Prêdkoœæ"),
	   WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
	   0, 30, 150, 30, hWnd, (HMENU)ID_RBUTTON2, GetModuleHandle(NULL), NULL);
   hwndButton = CreateWindow(TEXT("button"), TEXT("Droga"),
	   WS_CHILD | WS_VISIBLE | BS_CHECKBOX,
	   0, 60, 150, 30, hWnd, (HMENU)ID_RBUTTON3, GetModuleHandle(NULL), NULL);

   //Zmienianie amplitudy i podzia³ki czasowej


   HWND groupBox = CreateWindow(TEXT("BUTTON"), TEXT("Amplituda"),
	   WS_CHILD | WS_VISIBLE | BS_GROUPBOX | WS_GROUP,
	   10, 100, 143, 100,
	   hWnd, NULL, hInstance, 0);
   hwndButton = CreateWindow(TEXT("button"), TEXT("+"),
	   WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
	   26, 134, 30, 30, hWnd , (HMENU)ID_BUTTON1, hInstance, NULL);
   hwndButton = CreateWindow(TEXT("button"), TEXT("-"),
	   WS_CHILD | WS_VISIBLE,
	   81, 134, 30, 30, hWnd, (HMENU)ID_BUTTON2, GetModuleHandle(NULL), NULL);

   HWND groupBox1 = CreateWindow(TEXT("BUTTON"), TEXT("Podzia³ka czasowa"),
	   WS_CHILD | WS_VISIBLE | BS_GROUPBOX | WS_GROUP,
	   10, 210, 143, 100,
	   hWnd, NULL, hInstance, 0);
   hwndButton = CreateWindow(TEXT("button"), TEXT("+"),
	   WS_CHILD | WS_VISIBLE,
	   26, 245, 30, 30, hWnd, (HMENU)ID_BUTTON3, GetModuleHandle(NULL), NULL);
   hwndButton = CreateWindow(TEXT("button"), TEXT("-"),
	   WS_CHILD | WS_VISIBLE,
	   81, 245, 30, 30, hWnd, (HMENU)ID_BUTTON4, GetModuleHandle(NULL), NULL);

   hwndButton = CreateWindow(TEXT("button"), TEXT("Dalej"),
	   WS_CHILD | WS_VISIBLE,
	   1000, 550, 100, 30, hWnd, (HMENU)ID_BUTTON5, GetModuleHandle(NULL), NULL);
   hwndButton = CreateWindow(TEXT("button"), TEXT("Wróæ"),
	   WS_CHILD | WS_VISIBLE,
	   200, 550, 100, 30, hWnd, (HMENU)ID_BUTTON6, GetModuleHandle(NULL), NULL);

   if (!hWnd)
   {
      return FALSE;
   }
   OnCreate(hWnd);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
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
        {
			BOOL bChecked1 = (IsDlgButtonChecked(hWnd, ID_RBUTTON1) == BST_CHECKED);
			BOOL bChecked2 = (IsDlgButtonChecked(hWnd, ID_RBUTTON2) == BST_CHECKED);
			BOOL bChecked3 = (IsDlgButtonChecked(hWnd, ID_RBUTTON3) == BST_CHECKED);
            wmId = LOWORD(wParam);
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
			case ID_RBUTTON1:
				if(bChecked1)CheckDlgButton(hWnd, ID_RBUTTON1, BST_UNCHECKED);
				else CheckDlgButton(hWnd, ID_RBUTTON1, BST_CHECKED);
				repaintWindow(hWnd, hdc, ps, &drawArea1, &drawArea2, !bChecked1, bChecked2, bChecked3);
				break;
			case ID_RBUTTON2:
				if (bChecked2) CheckDlgButton(hWnd, ID_RBUTTON2, BST_UNCHECKED);
				else CheckDlgButton(hWnd, ID_RBUTTON2, BST_CHECKED);
				repaintWindow(hWnd, hdc, ps, &drawArea1, &drawArea2, bChecked1, !bChecked2, bChecked3);
				break;
			case ID_RBUTTON3:
				if (bChecked3) CheckDlgButton(hWnd, ID_RBUTTON3, BST_UNCHECKED);
				else CheckDlgButton(hWnd, ID_RBUTTON3, BST_CHECKED); 
				repaintWindow(hWnd, hdc, ps, &drawArea1, &drawArea2, bChecked1, bChecked2, !bChecked3);
				break;

			case ID_BUTTON1:
				amplituda+=10; 
				repaintWindow(hWnd, hdc, ps, &drawArea1, &drawArea2, bChecked1, bChecked2, bChecked3);
				break;
			case ID_BUTTON2:
				if (amplituda > 10)
				{
					amplituda -= 10;
					repaintWindow(hWnd, hdc, ps, &drawArea1, &drawArea2, bChecked1, bChecked2, bChecked3);
				}
				else MessageBox(hWnd, TEXT("Nie mo¿na bardziej zmnieszyæ amplitudy!"), NULL, MB_OK);
				break;
			case ID_BUTTON3:
				podzialka_czasu++;
				repaintWindow(hWnd, hdc, ps, &drawArea1, &drawArea2, bChecked1, bChecked2, bChecked3);
				break;
			case ID_BUTTON4:
				if(podzialka_czasu <= 1) MessageBox(hWnd, TEXT("Czas nie mo¿e byæ ujemny!"), NULL, MB_OK);
				else
				{
					podzialka_czasu--;
					repaintWindow(hWnd, hdc, ps, &drawArea1, &drawArea2, bChecked1, bChecked2, bChecked3);
				}
				break;
			case ID_BUTTON5:
				dalej += 500/podzialka_czasu;
				repaintWindow(hWnd, hdc, ps, &drawArea1, &drawArea2, bChecked1, bChecked2, bChecked3);
				break;
			case ID_BUTTON6:
				if (dalej >= 500/podzialka_czasu)
				{
					dalej -= 500/podzialka_czasu;
					repaintWindow(hWnd, hdc, ps, &drawArea1, &drawArea2, bChecked1, bChecked2, bChecked3);
				}
				break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
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
