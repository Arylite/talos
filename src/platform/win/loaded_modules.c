#include "platform/talos_platform.h"
#include "platform/win/module_enum.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdlib.h>
#include <string.h>

static int get_directory_of(const char *file_path, char *out_dir, size_t out_dir_size)
{
    strncpy(out_dir, file_path, out_dir_size - 1);
    out_dir[out_dir_size - 1] = '\0';

    char *last_slash = strrchr(out_dir, '\\');
    if (last_slash == NULL) {
        return 0;
    }

    *last_slash = '\0';
    return 1;
}

static int path_is_within(const char *path, const char *dir)
{
    size_t dir_len = strlen(dir);
    return _strnicmp(path, dir, dir_len) == 0;
}

talos_status talos_platform_check_loaded_modules(talos_signal_state *out_state)
{
    char exe_path[MAX_PATH];
    if (GetModuleFileNameA(NULL, exe_path, sizeof(exe_path)) == 0) {
        return TALOS_ERROR_UNKNOWN;
    }

    char exe_dir[MAX_PATH];
    if (!get_directory_of(exe_path, exe_dir, sizeof(exe_dir))) {
        return TALOS_ERROR_UNKNOWN;
    }

    char system_dir[MAX_PATH];
    char windows_dir[MAX_PATH];
    if (GetSystemDirectoryA(system_dir, sizeof(system_dir)) == 0 ||
        GetWindowsDirectoryA(windows_dir, sizeof(windows_dir)) == 0) {
        return TALOS_ERROR_UNKNOWN;
    }

    talos_win_module_info *modules = malloc(TALOS_WIN_MAX_MODULES * sizeof(*modules));
    if (modules == NULL) {
        return TALOS_ERROR_OUT_OF_MEMORY;
    }

    size_t module_count = 0;
    if (!talos_win_enumerate_modules(modules, &module_count)) {
        free(modules);
        return TALOS_ERROR_UNKNOWN;
    }

    talos_signal_state result = TALOS_SIGNAL_CLEAR;
    for (size_t i = 0; i < module_count; i++) {
        const char *path = modules[i].path;
        if (!path_is_within(path, exe_dir) &&
            !path_is_within(path, system_dir) &&
            !path_is_within(path, windows_dir)) {
            result = TALOS_SIGNAL_DETECTED;
            break;
        }
    }

    free(modules);
    *out_state = result;
    return TALOS_OK;
}
