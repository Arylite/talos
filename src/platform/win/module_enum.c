#include "platform/win/module_enum.h"

#define WIN32_LEAN_AND_MEAN
#include <tlhelp32.h>

#include <string.h>

int talos_win_enumerate_modules(talos_win_module_info *out_modules, size_t *out_count)
{
    *out_count = 0;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, GetCurrentProcessId());
    if (snapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    MODULEENTRY32 entry;
    entry.dwSize = sizeof(entry);

    size_t count = 0;
    if (Module32First(snapshot, &entry)) {
        do {
            if (count >= TALOS_WIN_MAX_MODULES) {
                break;
            }

            talos_win_module_info *info = &out_modules[count];
            strncpy(info->path, entry.szExePath, sizeof(info->path) - 1);
            info->path[sizeof(info->path) - 1] = '\0';
            info->base = (uintptr_t)entry.modBaseAddr;
            info->size = (size_t)entry.modBaseSize;
            count++;
        } while (Module32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);

    *out_count = count;
    return 1;
}
