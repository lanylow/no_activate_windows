#include <windows.h>
#include <stdbool.h>

int(__stdcall* get_window_band)(HWND, unsigned long*) = 0;
int(__stdcall* set_window_band)(HWND, unsigned long) = 0;

WNDPROC original_window_proc = 0;
HWND original_window = 0;

long long __stdcall window_proc(HWND window, unsigned int message, unsigned long long wparam, long long lparam) {
  if (message == WM_PAINT)
    return 0;

  return original_window_proc(window, message, wparam, lparam);
}

int __stdcall enum_child_windows(HWND window, long long lparam) {
  unsigned long band = 0;
  get_window_band(window, &band);

  if (band == 14) {
    original_window = window;
    original_window_proc = (WNDPROC)GetWindowLongPtrW(window, GWLP_WNDPROC);
    SetWindowLongPtrW(window, GWLP_WNDPROC, (long long)(window_proc));

    return false;
  }

  return true;
}

unsigned long __stdcall main_thread() {
  HMODULE user32 = LoadLibraryA("user32.dll");
  if (!user32)
    return 0;

  get_window_band = (void*)GetProcAddress(user32, "GetWindowBand");
  set_window_band = (void*)GetProcAddress(user32, "SetWindowBand");

  EnumChildWindows(GetDesktopWindow(), &enum_child_windows, 0);

  while (true) {
    long long status = GetWindowLongPtrW(original_window, GWL_STYLE);
    if (!status)
      break;

    if (status & WS_VISIBLE) {
      set_window_band(original_window, 1);
      ShowWindow(original_window, SW_HIDE);
      InvalidateRect(original_window, 0, true);
    }

    Sleep(1000);
  }
}

bool __stdcall DllMain(void* instance, unsigned long reason, void* reserved) {
  if (reason != DLL_PROCESS_ATTACH)
    return true;

  CreateThread(0, 0, (LPTHREAD_START_ROUTINE)(main_thread), 0, 0, 0);

  return true;
}