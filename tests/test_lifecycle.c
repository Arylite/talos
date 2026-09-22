#include "talos/talos.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static int g_log_call_count = 0;
static char g_last_log_message[256];

static void TALOS_CALL record_log(talos_log_level level, const char *message, void *user_data)
{
    (void)level;
    (void)user_data;

    g_log_call_count++;
    strncpy(g_last_log_message, message, sizeof(g_last_log_message) - 1);
    g_last_log_message[sizeof(g_last_log_message) - 1] = '\0';
}

static int is_valid_state(talos_signal_state state)
{
    return state == TALOS_SIGNAL_CLEAR || state == TALOS_SIGNAL_DETECTED || state == TALOS_SIGNAL_UNKNOWN;
}

static void test_individual_checks(talos_context *context)
{
    talos_signal_state state;

    assert(talos_check_debugger_present(context, &state) == TALOS_OK);
    assert(is_valid_state(state));
    assert(talos_check_debugger_present(NULL, &state) == TALOS_ERROR_INVALID_ARGUMENT);
    assert(talos_check_debugger_present(context, NULL) == TALOS_ERROR_INVALID_ARGUMENT);

    assert(talos_check_parent_process(context, &state) == TALOS_OK);
    assert(is_valid_state(state));

    assert(talos_check_hardware_breakpoints(context, &state) == TALOS_OK);
    assert(is_valid_state(state));

    assert(talos_check_loaded_modules(context, &state) == TALOS_OK);
    assert(is_valid_state(state));

    assert(talos_check_hidden_threads(context, &state) == TALOS_OK);
    assert(is_valid_state(state));

    assert(talos_check_api_hooks(context, &state) == TALOS_OK);
    assert(is_valid_state(state));

    assert(talos_check_self_integrity(context, &state) == TALOS_OK);
    assert(is_valid_state(state));
}

static void test_scan(talos_context *context)
{
    talos_scan_report report;
    assert(talos_scan(context, &report) == TALOS_OK);
    assert(report.count == 7);

    for (uint32_t i = 0; i < report.count; i++) {
        assert(report.checks[i].name != NULL);
        assert(report.checks[i].status == TALOS_OK);
        assert(is_valid_state(report.checks[i].state));
    }

    assert(talos_scan(NULL, &report) == TALOS_ERROR_INVALID_ARGUMENT);
    assert(talos_scan(context, NULL) == TALOS_ERROR_INVALID_ARGUMENT);
}

static void test_bypass(void)
{
    talos_config bypass_config;
    bypass_config.disable_checks = 1;

    talos_context *context = NULL;
    assert(talos_init(&bypass_config, &context) == TALOS_OK);

    talos_scan_report report;
    assert(talos_scan(context, &report) == TALOS_OK);
    for (uint32_t i = 0; i < report.count; i++) {
        assert(report.checks[i].status == TALOS_OK);
        assert(report.checks[i].state == TALOS_SIGNAL_CLEAR);
    }

    talos_shutdown(context);
}

static void test_logging(void)
{
    talos_config bypass_config;
    bypass_config.disable_checks = 1;

    talos_context *context = NULL;
    assert(talos_init(&bypass_config, &context) == TALOS_OK);

    talos_set_log_callback(context, record_log, NULL);

    g_log_call_count = 0;
    talos_signal_state state;
    assert(talos_check_debugger_present(context, &state) == TALOS_OK);
    assert(g_log_call_count == 1);
    assert(strstr(g_last_log_message, "bypassed") != NULL);

    talos_set_log_callback(context, NULL, NULL);
    g_log_call_count = 0;
    assert(talos_check_debugger_present(context, &state) == TALOS_OK);
    assert(g_log_call_count == 0);

    talos_shutdown(context);
}

int main(void)
{
    talos_context *context = NULL;
    assert(talos_init(NULL, &context) == TALOS_OK);
    assert(context != NULL);

    test_individual_checks(context);
    test_scan(context);

    talos_shutdown(context);

    test_bypass();
    test_logging();

    assert(talos_init(NULL, NULL) == TALOS_ERROR_INVALID_ARGUMENT);

    printf("talos version: %s\n", talos_version());
    printf("all tests passed\n");

    return 0;
}
