#include "platform/talos_platform.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

talos_status talos_platform_check_debugger_present(talos_signal_state *out_state)
{
    if (IsDebuggerPresent()) {
        *out_state = TALOS_SIGNAL_DETECTED;
        return TALOS_OK;
    }

    BOOL remote_present = FALSE;
    if (!CheckRemoteDebuggerPresent(GetCurrentProcess(), &remote_present)) {
        return TALOS_ERROR_UNKNOWN;
    }

    *out_state = remote_present ? TALOS_SIGNAL_DETECTED : TALOS_SIGNAL_CLEAR;
    return TALOS_OK;
}
