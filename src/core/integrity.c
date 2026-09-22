#include "talos/talos.h"
#include "talos_internal.h"
#include "checks_internal.h"
#include "platform/talos_platform.h"

#include <string.h>

TALOS_API talos_status TALOS_CALL talos_check_self_integrity(talos_context *context, talos_signal_state *out_state)
{
    talos_status status = talos_validate_check_args(context, out_state);
    if (status != TALOS_OK) {
        return status;
    }

    if (context->checks_disabled) {
        talos_log(context, TALOS_LOG_DEBUG, "self_integrity check bypassed by development configuration");
        *out_state = TALOS_SIGNAL_CLEAR;
        return TALOS_OK;
    }

    if (!context->integrity_baseline_valid) {
        *out_state = TALOS_SIGNAL_UNKNOWN;
        return TALOS_OK;
    }

    unsigned char current_hash[TALOS_HASH_SIZE];
    status = talos_platform_hash_self_code(current_hash);
    if (status != TALOS_OK) {
        talos_log(context, TALOS_LOG_WARNING, "self_integrity check query failed");
        return status;
    }

    *out_state = (memcmp(current_hash, context->integrity_baseline, TALOS_HASH_SIZE) == 0)
                     ? TALOS_SIGNAL_CLEAR
                     : TALOS_SIGNAL_DETECTED;
    return TALOS_OK;
}
