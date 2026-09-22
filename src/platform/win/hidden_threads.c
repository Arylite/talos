#include "platform/talos_platform.h"
#include "platform/win/module_enum.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>

#include <stdlib.h>

/*
 * A thread's Win32 start address is exposed only through the
 * undocumented NtQueryInformationThread / ThreadQuerySetWin32StartAddress
 * pair; there is no documented alternative. This is the same
 * technique long relied upon by mainstream diagnostic tools (Process
 * Explorer, Process Hacker) and has been stable across Windows
 * versions for over a decade.
 */
#define TALOS_THREADINFOCLASS_WIN32_START_ADDRESS 9

typedef LONG(WINAPI *NtQueryInformationThreadFn)(HANDLE, ULONG, PVOID, ULONG, PULONG);

static int get_thread_start_address(DWORD thread_id, PVOID *out_address)
{
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (ntdll == NULL) {
        return 0;
    }

    /*
     * GetProcAddress returns a generic FARPROC; converting it to our
     * specific signature via a union member access (rather than a
     * cast expression) avoids relying on an unsafe function-pointer
     * cast while still being well-defined for same-ABI Windows calls.
     */
    union {
        FARPROC proc;
        NtQueryInformationThreadFn fn;
    } converter;
    converter.proc = GetProcAddress(ntdll, "NtQueryInformationThread");
    if (converter.fn == NULL) {
        return 0;
    }

    NtQueryInformationThreadFn query_fn = converter.fn;

    HANDLE thread = OpenThread(THREAD_QUERY_LIMITED_INFORMATION, FALSE, thread_id);
    if (thread == NULL) {
        return 0;
    }

    PVOID start_address = NULL;
    LONG status = query_fn(thread, TALOS_THREADINFOCLASS_WIN32_START_ADDRESS,
                            &start_address, sizeof(start_address), NULL);
    CloseHandle(thread);

    if (status != 0) {
        return 0;
    }

    *out_address = start_address;
    return 1;
}

static int address_in_any_module(PVOID address, const talos_win_module_info *modules, size_t module_count)
{
    uintptr_t target = (uintptr_t)address;
    for (size_t i = 0; i < module_count; i++) {
        uintptr_t base = modules[i].base;
        if (target >= base && target < base + modules[i].size) {
            return 1;
        }
    }
    return 0;
}

talos_status talos_platform_check_hidden_threads(talos_signal_state *out_state)
{
    talos_win_module_info *modules = malloc(TALOS_WIN_MAX_MODULES * sizeof(*modules));
    if (modules == NULL) {
        return TALOS_ERROR_OUT_OF_MEMORY;
    }

    size_t module_count = 0;
    if (!talos_win_enumerate_modules(modules, &module_count)) {
        free(modules);
        return TALOS_ERROR_UNKNOWN;
    }

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        free(modules);
        return TALOS_ERROR_UNKNOWN;
    }

    DWORD current_pid = GetCurrentProcessId();
    THREADENTRY32 entry;
    entry.dwSize = sizeof(entry);

    talos_signal_state result = TALOS_SIGNAL_CLEAR;
    int any_resolved = 0;

    if (Thread32First(snapshot, &entry)) {
        do {
            if (entry.th32OwnerProcessID != current_pid) {
                continue;
            }

            PVOID start_address = NULL;
            if (!get_thread_start_address(entry.th32ThreadID, &start_address)) {
                continue;
            }

            any_resolved = 1;
            if (!address_in_any_module(start_address, modules, module_count)) {
                result = TALOS_SIGNAL_DETECTED;
                break;
            }
        } while (Thread32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);
    free(modules);

    *out_state = any_resolved ? result : TALOS_SIGNAL_UNKNOWN;
    return TALOS_OK;
}
