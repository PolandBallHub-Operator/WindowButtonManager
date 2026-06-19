#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <commctrl.h>
#include <gdiplus.h>
#include <vector>
#include <string>
#include <map>
#include <algorithm>

using namespace Gdiplus;

struct Config {
    int buttonSize = 24;
    int offsetX = 0;
    int offsetY = 0;
    int spacing = 4;
    wchar_t excludeList[1024] = L"SettingsClass,OverlayClass,#32768,Shell_TrayWnd,Progman";
} g_cfg;

ULONG_PTR gdiplusToken;
Image *g_imgClose = NULL, *g_imgMax = NULL, *g_imgMin = NULL;
std::map<HWND, HWND> g_overlays;
HINSTANCE g_hInst;

void LoadImages() {
    if (g_imgClose) delete g_imgClose; if (g_imgMax) delete g_imgMax; if (g_imgMin) delete g_imgMin;
    g_imgClose = new Image(L"assets/close.png"); g_imgMax = new Image(L"assets/max.png"); g_imgMin = new Image(L"assets/min.png");
}

void SaveConfig() {
    WritePrivateProfileString(L"Config", L"ButtonSize", std::to_wstring(g_cfg.buttonSize).c_str(), L"./config.ini");
    WritePrivateProfileString(L"Config", L"OffsetX", std::to_wstring(g_cfg.offsetX).c_str(), L"./config.ini");
    WritePrivateProfileString(L"Config", L"OffsetY", std::to_wstring(g_cfg.offsetY).c_str(), L"./config.ini");
    WritePrivateProfileString(L"Config", L"Spacing", std::to_wstring(g_cfg.spacing).c_str(), L"./config.ini");
    WritePrivateProfileString(L"Config", L"Exclude", g_cfg.excludeList, L"./config.ini");
}

void LoadConfig() {
    g_cfg.buttonSize = GetPrivateProfileInt(L"Config", L"ButtonSize", 24, L"./config.ini");
    g_cfg.offsetX = (int)GetPrivateProfileInt(L"Config", L"OffsetX", 0, L"./config.ini");
    g_cfg.offsetY = (int)GetPrivateProfileInt(L"Config", L"OffsetY", 0, L"./config.ini");
    g_cfg.spacing = (int)GetPrivateProfileInt(L"Config", L"Spacing", 4, L"./config.ini");
    GetPrivateProfileString(L"Config", L"Exclude", L"SettingsClass,OverlayClass,#32768,Shell_TrayWnd,Progman", g_cfg.excludeList, 1024, L"./config.ini");
}

LRESULT CALLBACK OverlayProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_PAINT) {
        PAINTSTRUCT ps; HDC hdc = BeginPaint(hwnd, &ps);
        int totalWidth = (g_cfg.buttonSize + g_cfg.spacing) * 3;
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc, totalWidth, g_cfg.buttonSize);
        SelectObject(memDC, memBitmap);
        Graphics graphics(memDC); graphics.Clear(Color(0, 0, 0, 0));
        if (g_imgClose) graphics.DrawImage(g_imgClose, 0, 0, g_cfg.buttonSize, g_cfg.buttonSize);
        if (g_imgMin) graphics.DrawImage(g_imgMin, g_cfg.buttonSize + g_cfg.spacing, 0, g_cfg.buttonSize, g_cfg.buttonSize);
        if (g_imgMax) graphics.DrawImage(g_imgMax, (g_cfg.buttonSize + g_cfg.spacing) * 2, 0, g_cfg.buttonSize, g_cfg.buttonSize);
        BLENDFUNCTION blend = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
        POINT ptSrc = { 0, 0 }; SIZE sizeWnd = { totalWidth, g_cfg.buttonSize };
        POINT ptDest = { 0, 0 }; RECT rect; GetWindowRect(hwnd, &rect); ptDest.x = rect.left; ptDest.y = rect.top;
        UpdateLayeredWindow(hwnd, hdc, &ptDest, &sizeWnd, memDC, &ptSrc, 0, &blend, ULW_ALPHA);
        DeleteObject(memBitmap); DeleteDC(memDC); EndPaint(hwnd, &ps);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

bool ShouldExclude(HWND hwnd) {
    wchar_t className[256]; GetClassName(hwnd, className, 256);
    std::wstring cls = className;
    std::wstring list = g_cfg.excludeList;
    if (list.find(cls) != std::wstring::npos) return true;
    
    LONG style = GetWindowLong(hwnd, GWL_STYLE);
    if (!(style & WS_CAPTION) || (style & WS_VISIBLE) == 0 || IsIconic(hwnd)) return true;
    
    // Check if it's a popup/menu (menus have #32768 class, but let's be safe)
    if (style & WS_POPUP) {
        // Most popups with buttons still have WS_SYSMENU
        if (!(style & WS_SYSMENU)) return true;
    }
    
    return false;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    if (ShouldExclude(hwnd)) {
        if (g_overlays.count(hwnd)) {
            ShowWindow(g_overlays[hwnd], SW_HIDE);
        }
        return TRUE;
    }

    TITLEBARINFOEX tbi = { sizeof(TITLEBARINFOEX) };
    if (SendMessage(hwnd, WM_GETTITLEBARINFOEX, 0, (LPARAM)&tbi)) {
        RECT closeRect = tbi.rgrect[5];
        if (closeRect.left != 0) {
            if (g_overlays.find(hwnd) == g_overlays.end()) {
                g_overlays[hwnd] = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT,
                    L"OverlayClass", NULL, WS_POPUP, 0, 0, 10, 10, NULL, NULL, g_hInst, NULL);
            }
            HWND overlay = g_overlays[hwnd];
            int totalWidth = (g_cfg.buttonSize + g_cfg.spacing) * 3;
            int x = closeRect.right - totalWidth + g_cfg.offsetX;
            int y = closeRect.top + g_cfg.offsetY;

            // Visibility Check via WindowFromPoint
            POINT pt = { x + totalWidth/2, y + g_cfg.buttonSize/2 };
            HWND found = WindowFromPoint(pt);
            bool visible = false;
            if (found == hwnd) visible = true;
            else {
                HWND p = found; while(p) { if(p==hwnd) {visible=true; break;} p=GetParent(p); }
            }

            if (visible) {
                SetWindowPos(overlay, HWND_TOPMOST, x, y, totalWidth, g_cfg.buttonSize, SWP_NOACTIVATE | SWP_SHOWWINDOW);
                InvalidateRect(overlay, NULL, FALSE);
            } else ShowWindow(overlay, SW_HIDE);
        }
    }
    return TRUE;
}

LRESULT CALLBACK SettingsProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE: {
            int y = 10;
            auto add_row = [&](const wchar_t* label, int id, std::wstring val) {
                CreateWindow(L"STATIC", label, WS_CHILD | WS_VISIBLE, 10, y, 80, 20, hwnd, NULL, g_hInst, NULL);
                CreateWindow(L"EDIT", val.c_str(), WS_CHILD | WS_VISIBLE | WS_BORDER, 100, y, 120, 20, hwnd, (HMENU)id, g_hInst, NULL);
                y += 30;
            };
            add_row(L"Size:", 101, std::to_wstring(g_cfg.buttonSize));
            add_row(L"Spacing:", 104, std::to_wstring(g_cfg.spacing));
            add_row(L"Offset X:", 102, std::to_wstring(g_cfg.offsetX));
            add_row(L"Offset Y:", 103, std::to_wstring(g_cfg.offsetY));
            add_row(L"Exclude:", 105, g_cfg.excludeList);
            CreateWindow(L"BUTTON", L"Apply & Save", WS_CHILD | WS_VISIBLE, 10, y, 210, 30, hwnd, (HMENU)201, g_hInst, NULL);
            return 0;
        }
        case WM_COMMAND:
            if (LOWORD(wParam) == 201) {
                wchar_t buf[1024];
                GetWindowText(GetDlgItem(hwnd, 101), buf, 16); g_cfg.buttonSize = _wtoi(buf);
                GetWindowText(GetDlgItem(hwnd, 102), buf, 16); g_cfg.offsetX = _wtoi(buf);
                GetWindowText(GetDlgItem(hwnd, 103), buf, 16); g_cfg.offsetY = _wtoi(buf);
                GetWindowText(GetDlgItem(hwnd, 104), buf, 16); g_cfg.spacing = _wtoi(buf);
                GetWindowText(GetDlgItem(hwnd, 105), g_cfg.excludeList, 1024);
                SaveConfig(); LoadImages();
                for (auto const& [t, o] : g_overlays) InvalidateRect(o, NULL, FALSE);
            }
            return 0;
        case WM_DESTROY: PostQuitMessage(0); return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    g_hInst = hInstance; GdiplusStartupInput gsi; GdiplusStartup(&gdiplusToken, &gsi, NULL);
    LoadConfig(); LoadImages();
    WNDCLASS oc = {0}; oc.lpfnWndProc = OverlayProc; oc.hInstance = hInstance; oc.lpszClassName = L"OverlayClass"; RegisterClass(&oc);
    WNDCLASS sc = {0}; sc.lpfnWndProc = SettingsProc; sc.hInstance = hInstance; sc.lpszClassName = L"SettingsClass"; sc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1); RegisterClass(&sc);
    HWND hSet = CreateWindow(L"SettingsClass", L"Window button manage NOADMIN", WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 250, 240, NULL, NULL, hInstance, NULL);
    ShowWindow(hSet, nCmdShow);
    MSG msg = {0};
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessage(&msg); }
        for (auto it = g_overlays.begin(); it != g_overlays.end(); ) {
            if (!IsWindow(it->first)) { DestroyWindow(it->second); it = g_overlays.erase(it); } else it++;
        }
        EnumWindows(EnumWindowsProc, 0);
        Sleep(10);
    }
    GdiplusShutdown(gdiplusToken); return 0;
}
