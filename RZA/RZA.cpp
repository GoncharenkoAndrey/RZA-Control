// RZA.cpp : Определяет точку входа для приложения.
//
#include <stdio.h>
#include <stdlib.h>
#include <thread>
#include "framework.h"
#include "RZA.h"
#include "modbus.h"
#include "ConfigWindow.h"
#include "Settings.h"

#define MAX_LOADSTRING 100
#define IDB_ONBUTTON 1
#define IDB_OFFBUTTON 2
#define IDB_RESETBUTTON 3

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];
HWND onButton, offButton, resetButton, iaText, ibText, icText;
Settings *settings;
char portString[50];
wchar_t portStringW[50];
HANDLE modbusThread, modbusMutex;
modbus_t* modbusContext;
struct networkData {
    float Uab, Ubc, Uca;
    float Ia, Ib, Ic;
};
networkData data;
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

DWORD WINAPI On(CONST LPVOID lParam) {
    modbus_set_slave(modbusContext, 1);
    modbus_connect(modbusContext);
    modbus_write_register(modbusContext, 0x80, 0b00001);
    modbus_close(modbusContext);
    return 0;
}
DWORD WINAPI Off(CONST LPVOID lParam) {
    modbus_set_slave(modbusContext, 1);
    modbus_connect(modbusContext);
    modbus_write_register(modbusContext, 0x80, 0b00010);
    modbus_close(modbusContext);
    return 0;
}
DWORD WINAPI Reset(CONST LPVOID lParam) {
    modbus_set_slave(modbusContext, 1);
    modbus_connect(modbusContext);
    modbus_write_register(modbusContext, 0x80, 0b00100);
    modbus_close(modbusContext);
    return 0;
}
DWORD WINAPI Read(CONST LPVOID lParam) {
    HANDLE hMutex = lParam;
    uint16_t value;
    int Ktt;
    double Ia, Ib, Ic;
    modbus_set_slave(modbusContext, 1);
    modbus_connect(modbusContext);
    modbus_read_registers(modbusContext, 0x23, 1, &value);
    Ktt = (int)value;
    modbus_read_registers(modbusContext, 0x50, 1, &value);
    Ia = value * Ktt * 0.01;
    modbus_read_registers(modbusContext, 0x51, 1, &value);
    Ib = value * Ktt * 0.01;
    modbus_read_registers(modbusContext, 0x52, 1, &value);
    Ic = value * Ktt * 0.01;
    data.Ia = Ia;
    data.Ib = Ib;
    data.Ic = Ic;
    modbus_close(modbusContext);
    ReleaseMutex(hMutex);
    return 0;
}
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    settings = Settings::getInstance();
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_RZA, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_RZA));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RZA));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_RZA);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
   
   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        size_t portLength;
        portLength = (size_t)lstrlen(portStringW);
        swprintf(portStringW, 15, L"\\\\.\\%s", settings->getPort());
        modbusContext = modbus_new_rtu(portString, settings->getSpeed(), 'N', 8, 1);
        iaText = CreateWindowW(L"STATIC", L"Ia: ", WS_VISIBLE | WS_CHILD, 5, 10, 100, 20, hWnd, NULL, hInst, NULL);
        ibText = CreateWindowW(L"STATIC", L"Ib: ", WS_VISIBLE | WS_CHILD, 5, 40, 100, 20, hWnd, NULL, hInst, NULL);
        icText = CreateWindowW(L"STATIC", L"Ic: ", WS_VISIBLE | WS_CHILD, 5, 70, 100, 20, hWnd, NULL, hInst, NULL);
        onButton = CreateWindowW(L"BUTTON", L"Включить", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 100, 100, 30, hWnd, (HMENU)IDB_ONBUTTON, hInst, NULL);
        offButton = CreateWindowW(L"BUTTON", L"Выключить", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 140, 100, 30, hWnd, (HMENU)IDB_OFFBUTTON, hInst, NULL);
        resetButton = CreateWindowW(L"BUTTON", L"Сброс", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
            10, 180, 100, 30, hWnd, (HMENU)IDB_RESETBUTTON, hInst, NULL);
        SetTimer(hWnd, 1, 5000, NULL);
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDB_ONBUTTON:
                CreateThread(NULL, 0, On, modbusMutex,0, NULL);
                break;
            case IDB_OFFBUTTON:
                CreateThread(NULL, 0, Off, modbusMutex, 0, NULL);
                break;
            case IDB_RESETBUTTON:
                CreateThread(NULL, 0, Reset, modbusMutex, 0, NULL);
                break;
            case IDM_DEVICE_PARAMETERS:
                ShowConfigWindow(hWnd, hInst);
                break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
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
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_TIMER:
        if (wParam == 1) {
            modbusMutex = CreateMutex(0, TRUE, 0);
            modbusThread = CreateThread(NULL, 0, Read, modbusMutex, 0, NULL);
            wchar_t text[15];
            WaitForSingleObject(modbusMutex, 10000);
            swprintf(text, 15, L"Ia: %.2f A", data.Ia);
            SetWindowText(iaText, text);
            swprintf(text, 15, L"Ib: %.2f A", data.Ib);
            SetWindowText(ibText, text);
            swprintf(text, 15, L"Ic: %.2f A", data.Ic);
            SetWindowText(icText, text);
            CloseHandle(modbusMutex);
            CloseHandle(modbusThread);
        }
        break;
    case WM_DESTROY:
        modbus_free(modbusContext);
        DestroyWindow(iaText);
        DestroyWindow(ibText);
        DestroyWindow(icText);
        DestroyWindow(onButton);
        DestroyWindow(offButton);
        DestroyWindow(resetButton);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Обработчик сообщений для окна "О программе".
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
