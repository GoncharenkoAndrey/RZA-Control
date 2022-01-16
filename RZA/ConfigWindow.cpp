#include <windows.h>
#include <stdio.h>
#include <CommCtrl.h>
#include "RZA.h"
#include "ConfigWindow.h"
#include "Settings.h"
#define IDB_SAVEBUTTON 1
#define IDB_CANCELBUTTON 2
HWND configWindow;
HWND portsListLabel;
HWND speedLabel;
HWND portsList;
HWND speedList;
HWND saveButton;
HWND cancelButton;
void SaveSettings() {
    wchar_t port[5];
    wchar_t speedText[10];
    int speed;
    int index = SendMessage(portsList, CB_GETCURSEL, 0, 0);
    SendMessage(portsList, CB_GETLBTEXT, index, (LPARAM)port);
    index = SendMessage(speedList, CB_GETCURSEL, 0, 0);
    SendMessage(speedList, CB_GETLBTEXT, index, (LPARAM)speedText);
    speed = _wtoi(speedText);
    Settings settings = *Settings::getInstance();
    settings.setPort(port);
    settings.setSpeed(speed);
}
LRESULT CALLBACK ConfigWindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_CREATE:
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId) {
            case IDB_SAVEBUTTON:
                SaveSettings();
                break;
            case IDB_CANCELBUTTON:
                CloseConfigWindow();
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
    }
        break;
    case WM_DESTROY:
        CloseConfigWindow();
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
void ShowConfigWindow(HWND parent, HINSTANCE hInstance) {

    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = ConfigWindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_RZA));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_MENU + 1);
    wcex.lpszMenuName = L"";
    wcex.lpszClassName = L"RZAConfig";
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    RegisterClassExW(&wcex);

    configWindow = CreateWindow(L"RZAConfig", L"Parameters", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        300, 50, 500, 500, NULL, NULL, hInstance, NULL);
    ShowWindow(configWindow, TRUE);
    UpdateWindow(configWindow);
    portsListLabel = CreateWindow(TEXT("STATIC"), TEXT("Порт"),
        WS_CHILD | WS_VISIBLE, 20, 30, 100, 20, configWindow, NULL, hInstance, NULL);
    portsList = CreateWindow(WC_COMBOBOX, NULL,
        CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_OVERLAPPED | WS_CHILD | WS_VISIBLE | WS_BORDER,
        130, 30, 200, 100, configWindow, NULL, hInstance, NULL);
    speedLabel = CreateWindow(TEXT("STATIC"), TEXT("Скорость"),
        WS_CHILD | WS_VISIBLE, 20, 70, 100, 20, configWindow, NULL, hInstance, NULL);
    speedList = CreateWindow(WC_COMBOBOX, NULL,
        CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_OVERLAPPED | WS_CHILD | WS_VISIBLE | WS_BORDER,
        130, 70, 100, 150, configWindow, NULL, hInstance, NULL);
    saveButton = CreateWindow(TEXT("BUTTON"), TEXT("Сохранить"), WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        230, 420, 100, 30, configWindow, (HMENU)IDB_SAVEBUTTON, hInstance, NULL);
    cancelButton = CreateWindow(TEXT("BUTTON"), TEXT("Отмена"), WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        360, 420, 100, 30, configWindow, (HMENU)IDB_CANCELBUTTON, hInstance, NULL);
    wchar_t port[20];
    HKEY key;
    DWORD l = 20;
    DWORD cValues;
    DWORD cchMaxValue;
    DWORD cchMaxValueData;
    DWORD retCode;
    retCode = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Hardware\\DeviceMap\\SerialComm"), 0, KEY_READ, &key);
    if (retCode == ERROR_SUCCESS) {
        RegQueryInfoKey(key, NULL, NULL, NULL, NULL, NULL, NULL, &cValues, &cchMaxValue, &cchMaxValueData, NULL, NULL);
        if (cValues) {
            for (DWORD i = 0; retCode == ERROR_SUCCESS, i < cValues; i++) {
                port[0] = '\0';
                retCode = RegEnumValue(key, i, port, &l, NULL, NULL, (LPBYTE)port, &l);
                SendMessage(portsList, CB_ADDSTRING, 0, (LPARAM)port);
            }
        }
    }
    SendMessage(portsList, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);
    SendMessage(speedList, CB_ADDSTRING, 0, (LPARAM)TEXT("9600"));
    SendMessage(speedList, CB_ADDSTRING, 0, (LPARAM)TEXT("19200"));
    SendMessage(speedList, CB_ADDSTRING, 0, (LPARAM)TEXT("38400"));
    SendMessage(speedList, CB_ADDSTRING, 0, (LPARAM)TEXT("57600"));
    SendMessage(speedList, CB_ADDSTRING, 0, (LPARAM)TEXT("115200"));
    SendMessage(speedList, CB_SETCURSEL, (WPARAM)1, (LPARAM) 0);
}
void CloseConfigWindow() {
    DestroyWindow(portsListLabel);
    DestroyWindow(portsList);
    DestroyWindow(speedLabel);
    DestroyWindow(speedList);
	CloseWindow(configWindow);
	DestroyWindow(configWindow);
}
