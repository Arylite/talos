#ifndef TALOS_H
#define TALOS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
#  if defined(TALOS_BUILD_SHARED)
#    define TALOS_API __declspec(dllexport)
#  elif defined(TALOS_USE_SHARED)
#    define TALOS_API __declspec(dllimport)
#  else
#    define TALOS_API
#  endif
#else
#  define TALOS_API
#endif

#define TALOS_CALL __cdecl

#define TALOS_VERSION_MAJOR 0
#define TALOS_VERSION_MINOR 3
#define TALOS_VERSION_PATCH 0
#define TALOS_VERSION_STRING "0.3.0"

/* Maximum number of entries talos_scan() can report in one call. */
#define TALOS_MAX_CHECKS 16

/*
 * Opaque handle to a Talos instance. Ownership is acquired via
 * talos_init() and released via talos_shutdown(). Callers must not
 * access its contents directly.
 */
typedef struct talos_context talos_context;

typedef enum talos_status {
    TALOS_OK = 0,
    TALOS_ERROR_UNKNOWN = 1,
    TALOS_ERROR_INVALID_ARGUMENT = 2,
    TALOS_ERROR_OUT_OF_MEMORY = 3
} talos_status;

/*
 * Result of an individual detection signal. A DETECTED result is one
 * independent data point, not a verdict: legitimate tools (IDEs,
 * profilers, overlays) can trigger the same signal. Callers should
 * combine multiple signals rather than act on any single one alone.
 *
 * UNKNOWN means the check ran without error but could not reach a
 * conclusion (for example, a race that made its result ambiguous).
 * Treat it as "no information", not as a positive or negative result.
 */
typedef enum talos_signal_state {
    TALOS_SIGNAL_CLEAR = 0,
    TALOS_SIGNAL_DETECTED = 1,
    TALOS_SIGNAL_UNKNOWN = 2
} talos_signal_state;

/*
 * Configuration supplied to talos_init(). Zero-initialize (or pass a
 * NULL config pointer) to get production defaults.
 */
typedef struct talos_config {
    /*
     * Non-zero makes every detection check report TALOS_SIGNAL_CLEAR
     * without performing its underlying query. Intended for
     * development and testing only; a production build must not ship
     * with this enabled.
     */
    int32_t disable_checks;
} talos_config;

/*
 * Severity of a diagnostic message passed to a talos_log_callback.
 */
typedef enum talos_log_level {
    TALOS_LOG_DEBUG = 0,
    TALOS_LOG_INFO = 1,
    TALOS_LOG_WARNING = 2,
    TALOS_LOG_ERROR = 3
} talos_log_level;

/*
 * Host-supplied diagnostic sink. message is a null-terminated,
 * UTF-8-ish string valid only for the duration of the call; copy it
 * if it must outlive the callback. May be invoked from any thread
 * that calls into Talos. Must not call back into Talos.
 */
typedef void (TALOS_CALL *talos_log_callback)(talos_log_level level, const char *message, void *user_data);

/*
 * Result of a single check as reported by talos_scan().
 *
 * name points to a static, null-terminated string valid for the
 * lifetime of the process. status reports whether the check itself
 * completed successfully; state is only meaningful when status is
 * TALOS_OK.
 */
typedef struct talos_check_result {
    const char *name;
    talos_status status;
    talos_signal_state state;
} talos_check_result;

/*
 * Batched results of every registered check, as produced by
 * talos_scan(). This is a convenience for running checks together;
 * it deliberately carries no combined verdict. Callers interpret
 * `checks[0..count)` themselves.
 */
typedef struct talos_scan_report {
    talos_check_result checks[TALOS_MAX_CHECKS];
    uint32_t count;
} talos_scan_report;

/*
 * Creates a new Talos instance and writes its handle to *out_context.
 * config may be NULL to use production defaults. The caller owns the
 * returned handle and must release it with talos_shutdown(). Not
 * thread-safe with respect to other calls operating on the same
 * context.
 */
TALOS_API talos_status TALOS_CALL talos_init(const talos_config *config, talos_context **out_context);

/*
 * Releases a Talos instance previously created by talos_init().
 * Accepts NULL as a no-op. The context must not be used after this
 * call returns.
 */
TALOS_API void TALOS_CALL talos_shutdown(talos_context *context);

/*
 * Returns a pointer to a static, null-terminated version string.
 * The returned pointer is valid for the lifetime of the process and
 * must not be freed by the caller.
 */
TALOS_API const char *TALOS_CALL talos_version(void);

/*
 * Registers a diagnostic callback on context, replacing any previous
 * one. Pass a NULL callback to stop receiving diagnostics (the
 * default). Talos stays silent unless a callback is registered; this
 * has no effect on detection behavior. Not thread-safe with respect
 * to other calls operating on the same context.
 */
TALOS_API void TALOS_CALL talos_set_log_callback(talos_context *context, talos_log_callback callback, void *user_data);

/*
 * Reports whether a user-mode debugger is currently attached to this
 * process (local or remote). Performs no allocation and is safe to
 * call concurrently from multiple threads. Returns an error status,
 * rather than a signal, only when the underlying platform query
 * itself fails.
 */
TALOS_API talos_status TALOS_CALL talos_check_debugger_present(talos_context *context, talos_signal_state *out_state);

/*
 * Reports whether this process's parent matches a small set of
 * expected launchers (Explorer, Steam, and common shells). This is a
 * coarse allow-list: legitimate launchers not on it (game store
 * clients, custom shortcuts, IDE debug launches) will read as
 * DETECTED, and it says nothing once a bypass tool has already
 * reparented itself as one of those launchers. Reports UNKNOWN, not
 * DETECTED, when the parent process could not be identified
 * conclusively (for example, its PID was reused by a newer process by
 * the time it was inspected). Performs no allocation.
 */
TALOS_API talos_status TALOS_CALL talos_check_parent_process(talos_context *context, talos_signal_state *out_state);

/*
 * Reports whether any of the current thread's hardware breakpoint
 * (debug register) slots are in use, which a software-only debugger
 * check can miss. Performs no allocation. Only inspects the calling
 * thread; other threads with hardware breakpoints set are not seen by
 * this call.
 */
TALOS_API talos_status TALOS_CALL talos_check_hardware_breakpoints(talos_context *context, talos_signal_state *out_state);

/*
 * Reports whether any module currently loaded in this process sits
 * outside the game's own install directory and the Windows system
 * directories. This is a coarse heuristic with a real false-positive
 * rate: legitimate overlays, accessibility tools, and some redistributed
 * runtimes load from other locations. Treat DETECTED as worth logging,
 * not as proof of anything. Allocates and frees a bounded internal
 * buffer for the duration of the call.
 */
TALOS_API talos_status TALOS_CALL talos_check_loaded_modules(talos_context *context, talos_signal_state *out_state);

/*
 * Reports whether this process has a thread whose start address does
 * not fall inside any currently loaded module, which is characteristic
 * of manually mapped or unbacked injected code. Legitimately injected
 * DLLs (overlays, accessibility tools) show up as loaded modules and
 * do not trigger this. Allocates and frees a bounded internal buffer
 * for the duration of the call.
 */
TALOS_API talos_status TALOS_CALL talos_check_hidden_threads(talos_context *context, talos_signal_state *out_state);

/*
 * Reports whether a small set of commonly-abused Win32 APIs (used to
 * enumerate processes, inject input, or intercept file/memory access)
 * show signs of inline hooking in this process. Not exhaustive by
 * design, and a forwarder-style jump that stays inside its own module
 * is not flagged. Reports UNKNOWN if none of the target modules were
 * loaded in this process to check. Performs no allocation.
 */
TALOS_API talos_status TALOS_CALL talos_check_api_hooks(talos_context *context, talos_signal_state *out_state);

/*
 * Reports whether the code of the module containing the Talos runtime
 * has changed since talos_init() computed a baseline hash for it (the
 * game executable for static builds, talos.dll for shared builds).
 * This only detects modification after that baseline was taken; it
 * cannot detect tampering already present at startup, and an attacker
 * who patches the code before talos_init() runs defeats it entirely.
 * Reports UNKNOWN if the baseline could not be computed at init time.
 */
TALOS_API talos_status TALOS_CALL talos_check_self_integrity(talos_context *context, talos_signal_state *out_state);

/*
 * Runs every check above and writes up to TALOS_MAX_CHECKS results
 * into out_report. This is a convenience for running checks together;
 * it does not produce, and must not be treated as, a combined
 * verdict. Each entry's own status/state must be interpreted
 * independently. Performs no allocation.
 */
TALOS_API talos_status TALOS_CALL talos_scan(talos_context *context, talos_scan_report *out_report);

#ifdef __cplusplus
}
#endif

#endif /* TALOS_H */
