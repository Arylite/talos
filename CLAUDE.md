# Talos

Talos is a small Windows user-mode anti-cheat library for indie games.

The purpose of Talos is deterrence and integrity. It should increase the cost of developing and maintaining simple cheats without attempting to provide impossible guarantees.

This document defines project constraints and engineering principles. It is not an implementation specification.

Claude should make reasonable architectural and implementation decisions within these constraints.

---

## Project constraints

* Target Windows x86-64.
* User-mode only.
* 100% offline.
* No network communication of any kind.
* No sockets.
* No HTTP/HTTPS.
* No DNS requests.
* No telemetry.
* No analytics.
* No remote logging.
* No cloud services.
* No external server dependency.
* No online activation.
* No license verification over the network.
* No update mechanism inside Talos.
* Do not introduce kernel drivers.
* Do not introduce hypervisors.
* Do not require DMA hardware.
* Do not depend on IOMMU functionality.
* Do not require virtualization for normal operation.
* Use C11 as the primary language.
* Use GCC / MinGW-w64.
* Use x86-64 assembly only when there is a concrete technical reason.
* Do not use C++.
* Keep external dependencies to a minimum.
* Prefer the Windows API and the C standard library.
* Keep the library usable independently of any game engine.

"100% offline" is a hard architectural requirement, not an optional configuration.

Talos must remain fully functional when:

* the machine has no network adapter
* networking is disabled
* DNS is unavailable
* the machine is behind a firewall
* the machine has no Internet connection

Talos must never require network connectivity to initialize, operate, perform checks, or report its local status.

Do not add network functionality even if it appears useful for future features.

If server-side validation is needed by the game, that belongs to the game architecture, not Talos.

Talos itself remains offline.

---

## Design philosophy

Talos is deliberately a small system.

Favor:

* simple designs
* explicit state
* clear ownership
* small interfaces
* isolated platform-specific code
* measurable behavior
* incremental work
* maintainable code

Avoid:

* unnecessary abstraction
* speculative infrastructure
* framework-heavy designs
* duplicated functionality
* opaque global state
* large subsystems without a demonstrated need

When two designs provide similar security properties, prefer the simpler one.

Do not optimize for how sophisticated the architecture looks.

---

## Security model

Assume that an attacker controls the machine running the game.

Assume that a sufficiently motivated attacker can:

* inspect the executable
* reverse engineer native code
* inspect process memory
* observe API usage
* patch client-side code
* modify local files
* use debugging tools
* interfere with user-mode execution

Do not treat client-side mechanisms as absolute security boundaries.

Do not claim that a technique makes something impossible to bypass unless that claim can actually be justified.

The objective is to increase attacker effort and reduce the usefulness of simple cheats.

Talos must achieve this entirely through local mechanisms.

---

## Detection philosophy

Detection should be based on multiple independent signals where practical.

Do not rely on a single check as an absolute indication of cheating.

Do not create a single centralized mechanism responsible for every detection.

Checks should be designed so that bypassing one mechanism does not automatically invalidate every other mechanism.

Use detection mechanisms only when their false-positive behavior and limitations are understood.

A suspicious signal should not automatically imply malicious behavior.

Legitimate software may interact with a game process in unusual ways.

All detection must be performed locally.

Talos must not send detection results to an external service.

Talos must not receive detection rules, configuration, signatures, or decisions from an external service at runtime.

---

## Client trust

Treat the client as untrusted.

Talos should protect client integrity where useful, but must not pretend that client-side secrets or checks are permanently trustworthy.

Important game state may be validated by the game server when the game itself uses networking.

However, Talos must remain completely independent from that networking.

Talos must not:

* contact the game server
* contact an authentication service
* contact a telemetry service
* contact an anti-cheat backend
* perform network-based validation

The game may consume Talos's local status and make its own decisions.

Do not automatically ban users based solely on a client-side signal.

---

## Performance

Talos must have a small runtime footprint.

Avoid expensive operations in hot paths.

Do not perform heavyweight process-wide operations every frame without a strong justification.

Prefer incremental or scheduled work when appropriate.

Avoid unnecessary allocations.

Avoid unnecessary threads.

Do not sacrifice game stability for marginal detection improvements.

Security mechanisms should be evaluated against their runtime cost.

---

## Platform isolation

Keep Windows-specific implementation details isolated from generic library logic.

Do not expose unnecessary platform-specific structures through the public API.

Prefer documented Windows interfaces when practical.

Undocumented interfaces require a concrete justification and careful consideration of compatibility.

Do not use lower-level APIs merely because they appear more sophisticated.

---

## Public API

The public interface should remain small and stable.

Prefer opaque types and explicit ownership.

Do not expose internal implementation structures.

The public API must use a stable C ABI.

Avoid exposing:

* C++ types
* compiler-specific types
* internal structures
* engine-specific types
* managed runtime types

Prefer standard C types and explicit buffer lengths.

Every public function must have clearly defined ownership, lifetime, error, and thread-safety behavior where relevant.

Do not expand the public API without a reason.

---

## Godot / C# integration

Talos may be embedded into games built with Godot and C#.

The Talos core must remain completely independent from Godot and .NET.

Godot integration must occur through a native C ABI suitable for P/Invoke.

The native interface must remain usable by non-Godot applications.

Do not put Godot-specific concepts into Talos core code.

Do not require .NET to build Talos.

The native implementation and the Godot integration layer should remain conceptually separate.

Claude should determine the appropriate build arrangement for producing a native library and a P/Invoke-compatible Windows interface while preserving these constraints.

The C# integration must not introduce any network functionality into Talos.

---

## ABI

The ABI is a compatibility boundary.

Treat it conservatively.

Prefer:

* fixed-width integer types
* `size_t`
* opaque pointers
* explicit buffers and lengths
* simple return values

Avoid ABI-sensitive data structures.

Exported functions must use a consistent calling convention.

Do not expose compiler- or language-runtime-specific implementation details.

---

## Memory

Memory ownership must always be explicit.

Every allocation must have a defined owner and lifetime.

Avoid unnecessary dynamic allocation.

Do not introduce a custom allocator unless the project develops a demonstrated need for one.

Do not hide expensive allocations behind ordinary-looking APIs.

Handle allocation failures deliberately.

---

## Error handling

Errors should be explicit and predictable.

Do not silently ignore failures that affect correctness or security.

When interacting with Windows APIs, preserve relevant error information before making calls that may overwrite it.

Do not turn recoverable conditions into unnecessary process termination.

Talos should prioritize maintaining the stability of the host game.

Network failure must never be an error condition because Talos must never perform network operations.

---

## Concurrency

Do not introduce concurrency unless it provides a meaningful benefit.

When concurrency is used:

* make ownership explicit
* define synchronization requirements
* avoid data races
* keep shared state minimal
* consider shutdown and lifetime carefully

Do not create background workers merely to make the architecture appear more advanced.

---

## Assembly

Assembly is permitted but deliberately discouraged unless justified.

Use it only when it provides a concrete advantage over C.

Keep assembly isolated and small.

Do not use assembly for ordinary logic.

Do not assume that assembly automatically provides security against reverse engineering.

Document assumptions about registers, calling conventions, and platform behavior when they are not obvious.

---

## Virtualization

A small internal virtual machine may be considered for a limited amount of sensitive logic.

It is not a core requirement.

Do not design Talos around a VM.

Do not move normal application logic into a VM merely for complexity.

If virtualization is eventually justified, keep its scope small and its implementation understandable.

The decision to introduce virtualization should be based on a concrete threat or bypass, not aesthetics.

The VM must execute entirely locally.

It must not depend on a remote interpreter, remote configuration, remote bytecode, or network service.

---

## Integrity

Integrity mechanisms should be selective and purposeful.

Do not blindly hash everything.

Do not assume that a local integrity check is inherently trustworthy.

Consider how an attacker could observe, patch, or bypass a mechanism before treating it as a meaningful security signal.

Prefer layered integrity checks where appropriate.

All integrity verification must be possible without contacting an external service.

---

## Process and memory inspection

Process, module, thread, memory, and debugger inspection may be used when they provide useful signals.

Do not automatically classify unusual system state as malicious.

Windows applications have many legitimate reasons for:

* dynamic libraries
* executable memory
* unusual threads
* debugging interfaces
* injected components
* shared memory
* runtime-generated code

Detection logic must account for legitimate behavior.

Do not destabilize the host process merely because an unusual condition exists.

---

## Logging and telemetry

Development diagnostics may be detailed.

Production behavior should remain minimal and privacy-conscious.

Talos must not implement telemetry.

Do not send logs anywhere.

Do not upload crash reports.

Do not transmit detection results.

Do not collect unrelated user information.

Local diagnostic output is acceptable when explicitly enabled by the host application.

---

## Cryptography

Do not implement custom cryptography.

Do not invent custom encryption, signatures, hashes, or key-exchange mechanisms when established primitives are available.

Do not assume that a secret embedded in the client remains secret.

Cryptography must not introduce network dependencies.

Cryptographic verification must work entirely offline.

---

## Development and production

Development builds must remain debuggable.

Do not make normal development unnecessarily difficult.

Provide appropriate mechanisms for developers to disable or bypass anti-cheat functionality during development and testing.

Production builds must not accidentally expose development bypasses.

Keep development functionality clearly separated from production behavior.

---

## Testing

Test behavior, not implementation details.

Important areas include:

* lifecycle
* invalid inputs
* failure paths
* resource cleanup
* ABI boundaries
* Windows API failures
* concurrency where applicable
* detection behavior
* legitimate edge cases
* performance-sensitive paths
* operation with networking completely disabled

Tests should verify that Talos behaves identically with or without network connectivity.

A test environment must be able to run Talos with:

* no Internet
* disabled network adapters
* blocked outbound connections
* unavailable DNS

Talos must continue to function normally.

Do not add tests that require external servers.

---

## Build system

The project should support a straightforward GCC / MinGW-w64 build.

A static native library should be available for native consumers.

A Windows dynamic library should be available when required for foreign-function interfaces such as C# P/Invoke.

The exact build organization is an implementation decision.

Keep build configuration understandable.

Do not duplicate configuration unnecessarily between build systems.

The build must not require network access after dependencies required for development have been installed.

Talos runtime operation must never require network access.

---

## Code style

Use conventional, readable C.

Prefer:

* 4 spaces
* K&R braces
* `snake_case`
* descriptive names
* small functions
* explicit control flow
* standard integer types

Public symbols should use a consistent `talos_` prefix.

Macros should use an appropriate `TALOS_` prefix.

Do not impose arbitrary complexity or artificial code structure simply to satisfy stylistic rules.

Readability takes precedence over cleverness.

---

## Comments

Keep comments to an absolute minimum.

Only add a comment when the code cannot reasonably communicate the intent, constraint, invariant, or non-obvious reason by itself.

Do not comment:

- obvious code
- function behavior already clear from its name
- implementation details that are self-explanatory
- trivial control flow
- standard Windows API usage
- code merely to increase documentation or coverage

Prefer clear names and simple code over explanatory comments.

Comments must describe why something is necessary, unusual, or constrained, not simply what the code does.

Never mention:

- AI
- Claude
- prompts
- conversations
- previous discussions
- previous versions
- previous states of the project
- instructions received during development
- generated code
- the reasoning process behind the implementation

Comments must be timeless and relevant to the code itself.

Avoid comments that will become stale when the implementation changes.

When in doubt, do not add the comment.

---

## Architecture decisions

Claude is expected to make implementation decisions.

Before introducing a significant subsystem, consider:

* the actual problem it solves
* the threat it addresses
* its limitations
* its runtime cost
* its complexity
* its maintenance cost
* its interaction with legitimate software
* whether a simpler mechanism would achieve the same goal
* whether the mechanism can remain completely offline

Do not implement speculative features merely because they may become useful later.

Do not create abstractions for hypothetical future requirements.

Do not introduce network functionality to solve a problem that can reasonably be solved locally.

---

## Scope control

Keep the project focused.

A feature should have a concrete purpose related to Talos.

Do not expand the project into:

* a general security framework
* a telemetry platform
* a networking framework
* an account-management system
* an online authentication system
* a cloud service
* a remote detection service
* a general-purpose virtualization platform

Talos is not responsible for communicating with game servers.

If networking is necessary for the game, it belongs to the game itself.

Talos remains offline.

---

## Engineering priorities

When making trade-offs, consider these properties:

1. Correctness
2. Host application stability
3. Maintainability
4. Meaningful security value
5. Performance
6. Simplicity
7. Offline operation

The offline requirement is absolute and must not be traded away for additional detection capability.

Do not sacrifice correctness or stability for superficial anti-cheat complexity.

---

## Working method

When implementing a feature:

1. Understand the existing architecture before changing it.
2. Inspect relevant code instead of assuming its structure.
3. Reuse existing mechanisms when appropriate.
4. Keep changes localized.
5. Avoid unrelated refactoring.
6. Build and test after meaningful changes.
7. Investigate failures instead of hiding them.
8. Prefer incremental implementation.
9. Keep the public API stable.
10. Document important architectural decisions when necessary.
11. Verify that new functionality does not introduce a network dependency.

Do not rewrite working subsystems without a concrete reason.

---

## Final principle

Talos should remain a small, native, understandable piece of software.

The implementation is deliberately left open.

Choose the simplest design that satisfies the requirements, survives realistic failure conditions, and provides meaningful additional cost to cheating.

Talos must remain completely functional without Internet access.

No runtime component may communicate over the network.

No detection decision may depend on an external service.

No telemetry may leave the machine.

Do not confuse complexity with security.

Do not confuse network connectivity with security.

Do not confuse sophistication with effectiveness.
