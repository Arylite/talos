#include "platform/talos_platform.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

talos_status talos_platform_check_hardware_breakpoints(talos_signal_state *out_state)
{
    CONTEXT context;
    ZeroMemory(&context, sizeof(context));
    context.ContextFlags = CONTEXT_DEBUG_REGISTERS;

    /*
     * Querying a thread's own debug registers via its pseudo handle is
     * not meaningful for general-purpose registers, but is a
     * long-standing, widely relied-upon technique specifically for the
     * debug register bank on Windows.
     */
    if (!GetThreadContext(GetCurrentThread(), &context)) {
        return TALOS_ERROR_UNKNOWN;
    }

    int in_use = (context.Dr0 != 0) || (context.Dr1 != 0) || (context.Dr2 != 0) || (context.Dr3 != 0);
    *out_state = in_use ? TALOS_SIGNAL_DETECTED : TALOS_SIGNAL_CLEAR;
    return TALOS_OK;
}
