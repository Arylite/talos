#include "platform/talos_platform.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <stdint.h>
#include <string.h>

typedef struct hooked_api_target {
    const char *module_name;
    const char *function_name;
} hooked_api_target;

/*
 * A deliberately short list of Win32 APIs commonly hooked by cheats
 * operating inside the game's own process: to hide their modules from
 * enumeration, to read/write memory, to inject input, or to intercept
 * file access. Not exhaustive by design — a much longer list would
 * grow the false-positive surface without meaningfully improving
 * detection.
 */
static const hooked_api_target k_targets[] = {
    { "kernel32.dll", "CreateFileW" },
    { "kernel32.dll", "VirtualProtect" },
    { "kernel32.dll", "CreateToolhelp32Snapshot" },
    { "user32.dll", "GetAsyncKeyState" },
    { "user32.dll", "SetWindowsHookExW" },
};

static int get_module_size(HMODULE module, size_t *out_size)
{
    BYTE *base = (BYTE *)module;
    IMAGE_DOS_HEADER *dos_header = (IMAGE_DOS_HEADER *)base;
    if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
        return 0;
    }

    IMAGE_NT_HEADERS *nt_headers = (IMAGE_NT_HEADERS *)(base + dos_header->e_lfanew);
    if (nt_headers->Signature != IMAGE_NT_SIGNATURE) {
        return 0;
    }

    *out_size = nt_headers->OptionalHeader.SizeOfImage;
    return 1;
}

static int address_in_range(uintptr_t address, uintptr_t base, size_t size)
{
    return address >= base && address < base + size;
}

static int prologue_indicates_hook(const unsigned char *bytes, uintptr_t function_address,
                                    uintptr_t module_base, size_t module_size)
{
    /*
     * FF 25 xx xx xx xx: jmp qword ptr [rip+disp32]. This is the
     * canonical trampoline shape emitted by hooking libraries
     * (Detours, MinHook); ordinary compiler-generated exports do not
     * take this form.
     */
    if (bytes[0] == 0xFF && bytes[1] == 0x25) {
        return 1;
    }

    /*
     * E9 xx xx xx xx: jmp rel32. Some legitimate exports are forwarder
     * thunks that jump to their real implementation inside the same
     * module, so only a jump that leaves the module is treated as
     * suspicious.
     */
    if (bytes[0] == 0xE9) {
        int32_t relative;
        memcpy(&relative, bytes + 1, sizeof(relative));
        uintptr_t target = function_address + 5 + (intptr_t)relative;
        return !address_in_range(target, module_base, module_size);
    }

    return 0;
}

talos_status talos_platform_check_api_hooks(talos_signal_state *out_state)
{
    talos_signal_state result = TALOS_SIGNAL_CLEAR;
    int any_checked = 0;

    size_t target_count = sizeof(k_targets) / sizeof(k_targets[0]);
    for (size_t i = 0; i < target_count; i++) {
        HMODULE module = GetModuleHandleA(k_targets[i].module_name);
        if (module == NULL) {
            continue;
        }

        FARPROC proc = GetProcAddress(module, k_targets[i].function_name);
        if (proc == NULL) {
            continue;
        }

        size_t module_size = 0;
        if (!get_module_size(module, &module_size)) {
            continue;
        }

        unsigned char prologue[6];
        memcpy(prologue, (const void *)(uintptr_t)proc, sizeof(prologue));

        any_checked = 1;

        if (prologue_indicates_hook(prologue, (uintptr_t)proc, (uintptr_t)module, module_size)) {
            result = TALOS_SIGNAL_DETECTED;
            break;
        }
    }

    *out_state = any_checked ? result : TALOS_SIGNAL_UNKNOWN;
    return TALOS_OK;
}
