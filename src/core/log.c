#include "talos/talos.h"
#include "talos_internal.h"

TALOS_API void TALOS_CALL talos_set_log_callback(talos_context *context, talos_log_callback callback, void *user_data)
{
    if (context == NULL) {
        return;
    }

    context->log_callback = callback;
    context->log_user_data = user_data;
}

void talos_log(const talos_context *context, talos_log_level level, const char *message)
{
    if (context == NULL || context->log_callback == NULL) {
        return;
    }

    context->log_callback(level, message, context->log_user_data);
}
