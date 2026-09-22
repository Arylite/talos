#ifndef TALOS_CHECKS_INTERNAL_H
#define TALOS_CHECKS_INTERNAL_H

#include "talos/talos.h"
#include "talos_internal.h"

/*
 * Validates the (context, out_state) pair every check function
 * receives. out_state is untyped so both talos_signal_state* and
 * other check-specific output pointers can share this check.
 */
talos_status talos_validate_check_args(const talos_context *context, const void *out_state);

typedef talos_status (*talos_platform_check_fn)(talos_signal_state *out_state);

/*
 * Shared body for a simple signal-state check: validates arguments,
 * honors context->checks_disabled, invokes platform_fn otherwise, and
 * logs on bypass or platform failure. check_name is used only for
 * diagnostics.
 */
talos_status talos_run_check(talos_context *context, talos_signal_state *out_state,
                              talos_platform_check_fn platform_fn, const char *check_name);

#endif /* TALOS_CHECKS_INTERNAL_H */
