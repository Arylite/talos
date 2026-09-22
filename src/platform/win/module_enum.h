#ifndef TALOS_WIN_MODULE_ENUM_H
#define TALOS_WIN_MODULE_ENUM_H

#include <stddef.h>
#include <stdint.h>
#include <windows.h>

#define TALOS_WIN_MAX_MODULES 512

typedef struct talos_win_module_info {
    char path[MAX_PATH];
    uintptr_t base;
    size_t size;
} talos_win_module_info;

/*
 * Snapshots the modules currently loaded in this process into
 * out_modules (capacity TALOS_WIN_MAX_MODULES entries) and writes the
 * number captured to *out_count. Extra modules beyond capacity are
 * silently dropped. Returns 0 on failure to snapshot at all.
 */
int talos_win_enumerate_modules(talos_win_module_info *out_modules, size_t *out_count);

#endif /* TALOS_WIN_MODULE_ENUM_H */
