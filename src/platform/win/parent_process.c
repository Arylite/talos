#include "platform/talos_platform.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <tlhelp32.h>

#include <string.h>

static const char *const k_expected_parents[] = {
    "explorer.exe",
    "steam.exe",
    "cmd.exe",
    "powershell.exe",
    "pwsh.exe",
    "windowsterminal.exe",
};

static int matches_expected_parent(const char *name)
{
    size_t count = sizeof(k_expected_parents) / sizeof(k_expected_parents[0]);
    for (size_t i = 0; i < count; i++) {
        if (_stricmp(name, k_expected_parents[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int find_process(HANDLE snapshot, DWORD pid, PROCESSENTRY32 *out_entry)
{
    out_entry->dwSize = sizeof(*out_entry);
    if (!Process32First(snapshot, out_entry)) {
        return 0;
    }

    do {
        if (out_entry->th32ProcessID == pid) {
            return 1;
        }
    } while (Process32Next(snapshot, out_entry));

    return 0;
}

static int get_process_creation_time(DWORD pid, ULONGLONG *out_ticks)
{
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (process == NULL) {
        return 0;
    }

    FILETIME creation_time, exit_time, kernel_time, user_time;
    BOOL ok = GetProcessTimes(process, &creation_time, &exit_time, &kernel_time, &user_time);
    CloseHandle(process);

    if (!ok) {
        return 0;
    }

    ULARGE_INTEGER ticks;
    ticks.LowPart = creation_time.dwLowDateTime;
    ticks.HighPart = creation_time.dwHighDateTime;
    *out_ticks = ticks.QuadPart;
    return 1;
}

talos_status talos_platform_check_parent_process(talos_signal_state *out_state)
{
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return TALOS_ERROR_UNKNOWN;
    }

    DWORD current_pid = GetCurrentProcessId();
    PROCESSENTRY32 self_entry;
    int found_self = find_process(snapshot, current_pid, &self_entry);

    PROCESSENTRY32 parent_entry;
    int found_parent = found_self && find_process(snapshot, self_entry.th32ParentProcessID, &parent_entry);

    CloseHandle(snapshot);

    if (!found_self || !found_parent) {
        *out_state = TALOS_SIGNAL_UNKNOWN;
        return TALOS_OK;
    }

    /*
     * The parent PID from the snapshot may have been reused by an
     * unrelated, newer process by the time we inspect it. A genuine
     * parent must have been created before this process; anything
     * else means we cannot trust the identity we just read.
     */
    ULONGLONG parent_creation, self_creation;
    if (!get_process_creation_time(parent_entry.th32ProcessID, &parent_creation) ||
        !get_process_creation_time(current_pid, &self_creation) ||
        parent_creation > self_creation) {
        *out_state = TALOS_SIGNAL_UNKNOWN;
        return TALOS_OK;
    }

    *out_state = matches_expected_parent(parent_entry.szExeFile) ? TALOS_SIGNAL_CLEAR : TALOS_SIGNAL_DETECTED;
    return TALOS_OK;
}
