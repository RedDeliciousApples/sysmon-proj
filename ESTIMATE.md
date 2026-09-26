# sysmon — embedded / Buildroot estimate

## Outcome

Turn sysmon from a Linux-host monitoring demo into a small, reproducible embedded
Linux service. The first target is a Buildroot-generated QEMU image; real hardware
is a separate follow-on because board support and boot media vary.

The first appliance is local-first: sysmon reads its own /proc data and exposes
/metrics on the target. Cloud transport is deferred until the local service is
reliable and observable.

## Current baseline

- C++17 application with CPU, memory, load average, uptime, and storage collectors.
- MetricsSampler can hold periodic snapshots, but must become the server data path.
- A small HTTP server and browser dashboard already exist.
- The Makefile has separate application/test targets, warnings, and pthread flags.
  It needs a cross-build cleanup pass, not a ground-up replacement.

The previous estimate duplicate-main finding is now stale: the active Makefile keeps
the sampler test out of application sources. Start from the current Makefile.

## Scope

### Included in the first embedded milestone

- Buildroot configuration committed to this repository.
- QEMU boot of a minimal Linux image that runs sysmon automatically.
- sysmon cross-compiled by Buildroot as an external package.
- Minimal runtime filesystem, init/service definition, and stable local endpoint.
- Host-side and QEMU smoke tests, plus a repeatable build/run guide.

### Deferred

- Cloud ingestion, TLS, authentication, fleet management, and OTA updates.
- Custom kernel/bootloader work and Internet-facing production hardening.
- Board-specific bring-up; estimate separately after choosing the board.

### Recommended target

Use QEMU virt on 64-bit ARM (aarch64) if the goal is a more embedded direction.
It uses a realistic cross toolchain and device-like image without tying the project
to a board. For the fastest first demo, QEMU x86_64 is also viable.

## Work breakdown

### Phase 0 — make the application ready for an appliance

| ID | Work | Estimate |
|----|------|---------:|
| C1 | Clean warnings; remove cross-build assumptions; make /proc failures explicit. | 3–4h |
| C2 | Complete snapshot-based serving and define first-sample/API behavior. | 2–4h |
| C3 | Make service shutdown deterministic; test lifecycle and recovery paths. | 2–4h |
| C4 | Make the dashboard truthful when the service is unavailable or warming up. | 2–3h |
| C5 | Add repeatable host smoke tests and operational documentation. | 2–3h |
| **Subtotal** | **Buildable, testable appliance baseline.** | **11–18h** |
#### C1 — warnings, portability, and safe collectors
- Keep the current warning set (`-Wall -Wextra -Wpedantic -Wshadow`) in both host and Buildroot builds; fix warnings rather than suppressing them.
- Do not hard-code the host compiler or flags in ways that override Buildroot's cross compiler. The package build must supply its target CXX, CPPFLAGS, CXXFLAGS, and LDFLAGS.
- In `src/collectors/mem.cpp`, initialize values and require all expected `MemTotal`, `MemFree`, and `MemAvailable` reads to succeed; reject zero `MemTotal` before dividing.
- In `loadavg.cpp` and `uptime.cpp`, check `fopen` and every `fscanf`, close descriptors on every outcome, and remove or repair the unused `get_uptime()` implementation.
- Treat a bad read as a collector error that the sampler logs and retains its last good snapshot for; never quietly publish zero, NaN, or uninitialized metrics.
#### C2 — sampler and HTTP contract
- Main already starts a `MetricsSampler` and passes it to `run_server`; retain that design and ensure `/metrics` never calls the slow collectors directly.
- Define the warming-up response: before the first snapshot, return a real 503 JSON error rather than the current empty successful response.
- Serialize the latest immutable snapshot, including an unset first CPU percentage, and keep `--watch` behavior separate from the HTTP data path.
- Example success condition: two rapid HTTP requests return promptly and report the same snapshot sequence until the next sampler interval.
#### C3 — lifecycle and failure behavior
- The sampler already uses a condition variable, so `stop()` can wake promptly; validate this with a timed test instead of assuming it.
- Add a controlled server shutdown path (for SIGTERM/SIGINT or a supplied stop flag) so the sampler destructor runs and the listening socket closes cleanly.
- Define recovery: if one `/proc` read fails, log the failure, keep the last good snapshot, and continue sampling on the next interval.
- Test start/stop/restart, no snapshot before first collection, one collector failure, and shutdown in less than one sampling interval.
#### C4 — dashboard truthfulness
- `frontend/app.js` currently fetches a hard-coded `http://localhost:8080/metrics` and falls back to invented mock values. Use relative `/metrics` so the UI works from the target device.
- Replace mock fallback with a visible “unreachable” or “stale” state, preserve the timestamp/sequence of the last good response, and distinguish 503 warming-up from a network failure.
- Align field names with the API; for example, choose `uptime_seconds` rather than keeping the old `awake` compatibility fallback indefinitely.
#### C5 — repeatable checks and handoff
- Add a host smoke command that starts sysmon, waits for `/metrics`, validates JSON/status, and stops the process without leaving it running.
- Run `make`, `make test`, and the smoke test with the same compiler options used by the package build; later repeat the HTTP assertion inside QEMU.
- Document `--serve`, `--watch`, default port/bind behavior, endpoint schema, expected 503 warming-up response, and log location.
**Completion check:** make, make test, and a local /metrics smoke test pass; the server returns a defined response at startup, does not wait a sampling interval per request, and stops cleanly.

### Phase 1 — package sysmon for Buildroot

| ID | Work | Estimate |
|----|------|---------:|
| B1 | Add external tree (external.desc, Config.in, package directory) and pin/document Buildroot version. | 2–4h |
| B2 | Write sysmon.mk and Config.in; cross-compile with Buildroot and install executable plus selected web assets. | 3–5h |
| B3 | Remove cross-build hazards: honor Buildroot flags, avoid host paths/tools, decide dynamic vs. static linking. | 2–4h |
| B4 | Add package build check and exact BR2_EXTERNAL build command to docs. | 1–2h |
| **Subtotal** | **Cross-built sysmon package.** | **8–15h** |

Completion check: a clean Buildroot build produces the target binary through the
package, not through a manually copied host build.

### Phase 2 — produce a runnable QEMU image

| ID | Work | Estimate |
|----|------|---------:|
| I1 | Minimal QEMU defconfig: architecture, C++ runtime, procfs/sysfs, networking, sysmon. | 3–5h |
| I2 | BusyBox init script or systemd unit; BusyBox init is recommended for the first small image. | 2–3h |
| I3 | Define port, bind address, logging, restart policy, and graceful stop. | 2–3h |
| I4 | Include only needed dashboard assets, or intentionally ship API-only first. | 1–3h |
| I5 | QEMU boot command, serial validation, network setup, and host access to /metrics. | 3–5h |
| **Subtotal** | **Bootable image with auto-starting sysmon.** | **11–19h** |

Completion check: the documented QEMU command boots the image; sysmon starts without
an interactive shell and the host can retrieve /metrics.

### Phase 3 — verification and workflow

| ID | Work | Estimate |
|----|------|---------:|
| V1 | Automate image build + boot smoke test: timeout, readiness, HTTP assertion. | 3–5h |
| V2 | Test first CPU sample, port conflicts, stop/restart, and practical /proc failure cases. | 2–4h |
| V3 | Measure image size/startup time; remove unnecessary packages and assets. | 2–4h |
| V4 | Rewrite README with prerequisites, reproducible Buildroot build, QEMU run, tests, architecture. | 2–3h |
| **Subtotal** | **Repeatable embedded development workflow.** | **9–16h** |

## Estimates by milestone

| Milestone | Scope | Estimate |
|-----------|-------|---------:|
| Application-ready | Phase 0 | 11–18h |
| Buildroot package | Phases 0–1 | 19–33h |
| QEMU embedded demo | Phases 0–2 | 30–52h |
| Reproducible QEMU workflow | Phases 0–3 | **39–68h** |

At 10 focused hours per week, the QEMU milestone is about 3–6 weeks. The wider range
allows for Buildroot/QEMU learning and package-debugging time.

## Hardware follow-on — not included

| Work | Estimate |
|------|---------:|
| Select board, validate Buildroot support, adjust defconfig | 3–8h |
| Boot media, bootloader/kernel configuration, serial console, first boot | 4–10h |
| Networking and service validation on device | 2–5h |
| Board-specific metric/storage quirks and documentation | 2–5h |
| **Typical board bring-up addition** | **11–28h** |

Do not fold this into the QEMU estimate: a supported Raspberry Pi, BeagleBone, or
custom ARM board can differ substantially in boot and driver work.

## Recommended execution order

1. Finish C1–C3. Snapshot serving and reliable shutdown matter more on an appliance
   than dashboard polish.
2. Add the external Buildroot tree and build sysmon as a package.
3. Create the smallest possible QEMU image with BusyBox init and an API-only service.
4. Prove automatic start, HTTP reachability, restart, and shutdown in QEMU.
5. Add dashboard assets after the service image works.
6. Automate the QEMU smoke test and document the workflow.
7. Choose physical hardware only when the QEMU milestone is stable.

## First concrete next steps

1. Pick a target: QEMU aarch64 virt is recommended; use x86_64 only if fastest
   initial feedback matters more than architecture practice.
2. Choose and pin a Buildroot release; create buildroot-external/.
3. Require make test and a local HTTP smoke test before building an image.
4. Convert the sampler to the server normal source of truth before packaging.
5. Add package/sysmon/, then build an image before cloud or real-board work.

## Risks to manage

- **Buildroot version drift:** pin the release and record it with the defconfig.
- **C++ runtime size:** start dynamically linked and measure before choosing static
  linking or changing libraries.
- **Frontend scope creep:** keep the first image API-first unless a dashboard is a
  core demo requirement.
- **Security assumptions:** exposing telemetry beyond a trusted local network needs
  separate authentication/TLS threat modeling.
- **Hardware uncertainty:** do not promise a board schedule before selecting the
  board, boot medium, and network setup.
