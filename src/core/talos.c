#include "talos/talos.h"
#include "talos_internal.h"
#include "platform/talos_platform.h"

#include <stdlib.h>

TALOS_API talos_status TALOS_CALL talos_init(const talos_config *config, talos_context **out_context)
{
    if (out_context == NULL) {
        return TALOS_ERROR_INVALID_ARGUMENT;
    }

    talos_context *context = malloc(sizeof(*context));
    if (context == NULL) {
        return TALOS_ERROR_OUT_OF_MEMORY;
    }

    context->initialized = 1;
    context->checks_disabled = (config != NULL) && (config->disable_checks != 0);
    context->log_callback = NULL;
    context->log_user_data = NULL;

    context->integrity_baseline_valid =
        (talos_platform_hash_self_code(context->integrity_baseline) == TALOS_OK) ? 1 : 0;

    *out_context = context;

    return TALOS_OK;
}

TALOS_API void TALOS_CALL talos_shutdown(talos_context *context)
{
    if (context == NULL) {
        return;
    }

    free(context);
}

TALOS_API const char *TALOS_CALL talos_version(void)
{
    return TALOS_VERSION_STRING;
}
