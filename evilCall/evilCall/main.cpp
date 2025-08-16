#include <windows.h>
#include <gdiplus.h>
#include <string>
#include <vector>
#include <iostream>

#pragma comment(lib, "Gdiplus.lib")

using namespace Gdiplus;

// Helper: get encoder CLSID by MIME
int GetEncoderClsid(const WCHAR* mimeType, CLSID* pClsid) {
    UINT num = 0, size = 0;
    if (GetImageEncodersSize(&num, &size) != Ok || size == 0) return -1;

    std::vector<BYTE> buffer(size);
    ImageCodecInfo* pInfo = reinterpret_cast<ImageCodecInfo*>(buffer.data());
    if (GetImageEncoders(num, size, pInfo) != Ok) return -1;

    for (UINT i = 0; i < num; ++i) {
        if (wcscmp(pInfo[i].MimeType, mimeType) == 0) {
            *pClsid = pInfo[i].Clsid;
            return static_cast<int>(i);
        }
    }
    return -1;
}

// Capture the entire virtual screen (all monitors) into a 32-bit top-down DIB
HBITMAP CaptureScreenDIB() {
    int vx = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int vy = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int vw = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int vh = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    if (vw <= 0 || vh <= 0) return nullptr;

    HDC hScreen = GetDC(nullptr);
    if (!hScreen) return nullptr;

    HDC hMem = CreateCompatibleDC(hScreen);
    if (!hMem) { ReleaseDC(nullptr, hScreen); return nullptr; }

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = vw;
    bmi.bmiHeader.biHeight = -vh;         // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;          // BGRA
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HBITMAP hbm = CreateDIBSection(hScreen, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!hbm) {
        DeleteDC(hMem);
        ReleaseDC(nullptr, hScreen);
        return nullptr;
    }

    HGDIOBJ old = SelectObject(hMem, hbm);
    BOOL bltOk = BitBlt(hMem, 0, 0, vw, vh, hScreen, vx, vy, SRCCOPY | CAPTUREBLT);
    SelectObject(hMem, old);

    DeleteDC(hMem);
    ReleaseDC(nullptr, hScreen);

    if (!bltOk) {
        DeleteObject(hbm);
        return nullptr;
    }
    return hbm;
}

// Save an HBITMAP as PNG via GDI+ (GDI+ must be initialized)
bool SaveHBITMAPAsPNG(HBITMAP hbm, const std::wstring& path) {
    if (!hbm) return false;

    // Wrap HBITMAP; ensure it's not selected in any DC at this point
    Bitmap bmp(hbm, nullptr);
    if (bmp.GetLastStatus() != Ok) {
        std::wcerr << L"Bitmap(HBITMAP) failed, status=" << bmp.GetLastStatus() << L"\n";
        return false;
    }

    CLSID pngClsid;
    if (GetEncoderClsid(L"image/png", &pngClsid) < 0) {
        std::wcerr << L"PNG encoder not found.\n";
        return false;
    }

    Status s = bmp.Save(path.c_str(), &pngClsid, nullptr);
    if (s != Ok) {
        std::wcerr << L"Bitmap::Save failed, status=" << s << L"\n";
        return false;
    }
    return true;
}

int wmain() {
    // Optional: avoid DPI scaling artifacts
    SetProcessDPIAware();

    // Initialize GDI+ once
    GdiplusStartupInput gpsi;
    ULONG_PTR token = 0;
    Status gs = GdiplusStartup(&token, &gpsi, nullptr);
    if (gs != Ok) {
        std::wcerr << L"GdiplusStartup failed, status=" << gs << L"\n";
        return 1;
    }

    // Capture
    HBITMAP hbm = CaptureScreenDIB();
    if (!hbm) {
        std::wcerr << L"Screen capture failed.\n";
        GdiplusShutdown(token);
        return 1;
    }

    // Save
    std::wstring outPath = L"test.png"; // change if you like
    bool saved = SaveHBITMAPAsPNG(hbm, outPath);

    // Clean up GDI object AFTER saving (Bitmap wrapper is already destroyed)
    DeleteObject(hbm);

    // Now it's safe to shut down GDI+
    GdiplusShutdown(token);

    if (!saved) {
        std::wcerr << L"Saving failed (see messages above).\n";
        return 1;
    }

    std::wcout << L"Saved screenshot to " << outPath << L"\n";
    return 0;
}
