#ifndef TALOS_PLATFORM_H
#define TALOS_PLATFORM_H

#include "talos/talos.h"

/*
 * Contract implemented once per supported platform. Keeps OS-specific
 * headers and calls out of the generic core logic in src/core.
 */

talos_status talos_platform_check_debugger_present(talos_signal_state *out_state);
talos_status talos_platform_check_parent_process(talos_signal_state *out_state);
talos_status talos_platform_check_hardware_breakpoints(talos_signal_state *out_state);
talos_status talos_platform_check_loaded_modules(talos_signal_state *out_state);
talos_status talos_platform_check_hidden_threads(talos_signal_state *out_state);
talos_status talos_platform_check_api_hooks(talos_signal_state *out_state);

/*
 * Computes a SHA-256 digest (32 bytes) of the code section of the
 * module containing the Talos runtime and writes it to out_hash.
 */
talos_status talos_platform_hash_self_code(unsigned char out_hash[32]);

#endif /* TALOS_PLATFORM_H */
