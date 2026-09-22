#ifndef TALOS_INTERNAL_H
#define TALOS_INTERNAL_H

#include "talos/talos.h"

/* SHA-256 digest size, used for the self-integrity baseline. */
#define TALOS_HASH_SIZE 32

struct talos_context {
    int initialized;
    int checks_disabled;

    talos_log_callback log_callback;
    void *log_user_data;

    int integrity_baseline_valid;
    unsigned char integrity_baseline[TALOS_HASH_SIZE];
};

/*
 * Forwards a diagnostic message to context's registered callback, if
 * any. No-op when context is NULL or has no callback registered.
 */
void talos_log(const talos_context *context, talos_log_level level, const char *message);

#endif /* TALOS_INTERNAL_H */
