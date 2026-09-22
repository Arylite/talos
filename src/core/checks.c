#include "talos/talos.h"
#include "talos_internal.h"
#include "checks_internal.h"
#include "platform/talos_platform.h"

#include <stdio.h>

talos_status talos_validate_check_args(const talos_context *context, const void *out_state)
{
    if (context == NULL || out_state == NULL) {
        return TALOS_ERROR_INVALID_ARGUMENT;
    }
    return TALOS_OK;
}

talos_status talos_run_check(talos_context *context, talos_signal_state *out_state,
                              talos_platform_check_fn platform_fn, const char *check_name)
{
    talos_status status = talos_validate_check_args(context, out_state);
    if (status != TALOS_OK) {
        return status;
    }

    if (context->checks_disabled) {
        char message[128];
        snprintf(message, sizeof(message), "%s check bypassed by development configuration", check_name);
        talos_log(context, TALOS_LOG_DEBUG, message);

        *out_state = TALOS_SIGNAL_CLEAR;
        return TALOS_OK;
    }

    status = platform_fn(out_state);
    if (status != TALOS_OK) {
        char message[128];
        snprintf(message, sizeof(message), "%s check query failed", check_name);
        talos_log(context, TALOS_LOG_WARNING, message);
    }

    return status;
}

TALOS_API talos_status TALOS_CALL talos_check_debugger_present(talos_context *context, talos_signal_state *out_state)
{
    return talos_run_check(context, out_state, talos_platform_check_debugger_present, "debugger_present");
}

TALOS_API talos_status TALOS_CALL talos_check_parent_process(talos_context *context, talos_signal_state *out_state)
{
    return talos_run_check(context, out_state, talos_platform_check_parent_process, "parent_process");
}

TALOS_API talos_status TALOS_CALL talos_check_hardware_breakpoints(talos_context *context, talos_signal_state *out_state)
{
    return talos_run_check(context, out_state, talos_platform_check_hardware_breakpoints, "hardware_breakpoints");
}

TALOS_API talos_status TALOS_CALL talos_check_loaded_modules(talos_context *context, talos_signal_state *out_state)
{
    return talos_run_check(context, out_state, talos_platform_check_loaded_modules, "loaded_modules");
}

TALOS_API talos_status TALOS_CALL talos_check_hidden_threads(talos_context *context, talos_signal_state *out_state)
{
    return talos_run_check(context, out_state, talos_platform_check_hidden_threads, "hidden_threads");
}

TALOS_API talos_status TALOS_CALL talos_check_api_hooks(talos_context *context, talos_signal_state *out_state)
{
    return talos_run_check(context, out_state, talos_platform_check_api_hooks, "api_hooks");
}

typedef talos_status(TALOS_CALL *talos_check_fn)(talos_context *, talos_signal_state *);

/*
 * Every public check, including ones (like self_integrity) that don't
 * fit talos_run_check's platform_fn shape. Each function already
 * performs its own argument validation, bypass handling, and logging.
 */
static const struct {
    const char *name;
    talos_check_fn fn;
} k_checks[] = {
    { "debugger_present", talos_check_debugger_present },
    { "parent_process", talos_check_parent_process },
    { "hardware_breakpoints", talos_check_hardware_breakpoints },
    { "loaded_modules", talos_check_loaded_modules },
    { "hidden_threads", talos_check_hidden_threads },
    { "api_hooks", talos_check_api_hooks },
    { "self_integrity", talos_check_self_integrity },
};

TALOS_API talos_status TALOS_CALL talos_scan(talos_context *context, talos_scan_report *out_report)
{
    talos_status status = talos_validate_check_args(context, out_report);
    if (status != TALOS_OK) {
        return status;
    }

    size_t check_count = sizeof(k_checks) / sizeof(k_checks[0]);
    if (check_count > TALOS_MAX_CHECKS) {
        check_count = TALOS_MAX_CHECKS;
    }

    for (size_t i = 0; i < check_count; i++) {
        talos_signal_state state = TALOS_SIGNAL_CLEAR;
        talos_status check_status = k_checks[i].fn(context, &state);

        out_report->checks[i].name = k_checks[i].name;
        out_report->checks[i].status = check_status;
        out_report->checks[i].state = state;
    }

    out_report->count = (uint32_t)check_count;
    return TALOS_OK;
}
