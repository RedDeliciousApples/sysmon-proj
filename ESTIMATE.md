# sysmon — Code-Only Estimate (pre-Buildroot)

Scope: everything needed to make the codebase "near perfect" before starting
Buildroot work. Excludes Buildroot/QEMU/bring-up (estimated separately at
~13–27h, unchanged).

## 1. Files reviewed

Trusted contents in chat: `Makefile`, `.gitignore`, `README.md`,
`include/metrics.hpp`, `src/main.cpp`, `src/utils/getjson.{h,cpp}`,
`src/utils/math_utils.{h,cpp}`, `src/collectors/{cpu,mem,loadavg,uptime,storage}.{h,cpp}`,
`src/collectors/metrics_sampler.{h,cpp}`, `src/collectors/metrics_snapshot.h`,
`src/server/http_server.{h,cpp}`, `frontend/{index.html,app.js,styles.css}`.

Not reviewed / N/A:
- `external/json.hpp` — vendored third-party; version not yet provided (not blocking)
- tests — none exist in the repo

## 2. Confirmed bugs (P0 — fix first)

| #  | File                        | Bug                                                                                                                                 | Est  |
|----|-----------------------------|-------------------------------------------------------------------------------------------------------------------------------------|------|
| B1 | `src/collectors/mem.cpp`    | `total`/`available` uninitialized; if `fscanf` fails, `round_to` computes on garbage → UB/NaN output                                 | 0.5h |
| B2 | `src/collectors/uptime.cpp` | `get_uptime()`: no `fopen` null check (UB crash), no `fclose` (leak), meaningless math, and it is dead code (nothing calls it)        | 0.5h |
| B3 | `src/main.cpp`              | `strcmp` used without `#include <cstring>` — compiles by luck via transitive includes; breaks on other toolchains/libc               | 0.1h |
| B4 | `src/server/http_server.cpp`| `send()` without `MSG_NOSIGNAL`: client disconnect mid-response raises SIGPIPE and kills the server                                  | 0.5h |
| B5 | `frontend/styles.css`       | Selector is `.body` (class) but `<body>` has no class → font rule never applies                                                      | 0.1h |

Fix for B2: delete `get_uptime()`, keep `time_awake()` (correct implementation),
remove duplicate/unused includes (`<stdexcept>` ×2, `<iostream>` ×2, `<chrono>`,
`<thread>`, `<utility>`, `<cmath>`).

## 3. Architecture: the sampler is built but never wired in

Current request path:
browser → http_server → `get_metrics_json()` → `get_cpu_usage()` →
**sleeps 1 second per request**, on a single-threaded accept loop.
Every `/metrics` request stalls ≥1s and blocks all other clients.

Meanwhile `MetricsSampler` (well-written: atomic stop flag, mutex-guarded
snapshot, delta-based CPU with no sleeping, exception-safe loop) is never
instantiated anywhere.

Target architecture:
- `main.cpp`: construct + start `MetricsSampler`; pass it to `run_server`
- `http_server.cpp`: `/metrics` serves `sampler.latest()` — non-blocking
- `getjson.cpp`: becomes a pure snapshot→JSON serializer
  (must handle `cpu_percent == nullopt` on the first sample)
- delete `include/metrics.hpp` (dead file; duplicates `getjson.h` + `cpu.h`)
- rename JSON key `"awake"` → `"uptime_seconds"` (matches the snapshot field;
  drop the frontend `??` fallback)
- `--watch` CLI mode keeps direct collector calls (blocking is fine there)

## 4. Punch list

### P1 — architecture + robustness
| #  | Item                                                                                     | File(s)                                              | Est       |
|----|------------------------------------------------------------------------------------------|------------------------------------------------------|-----------|
| A1 | Wire sampler into server (see §3)                                                        | main.cpp, http_server.{h,cpp}, getjson.cpp, delete metrics.hpp | 3.5–4.5h |
| A2 | `stop()` blocks up to `interval_` (`sleep_for` in loop); use condition_variable so stop is prompt | metrics_sampler.{h,cpp}                     | 1h        |
| A3 | `loadavg.cpp`: check `fscanf` returns (currently silently returns zeros on parse failure) | loadavg.cpp                                          | 0.5h      |
| A4 | Document the error policy: collectors may throw; sampler catches, logs, keeps last snapshot (already implemented — just comment it) | —    | 0.25h     |

### P2 — frontend honesty
| #  | Item                                                                 | File(s)            | Est       |
|----|----------------------------------------------------------------------|--------------------|-----------|
| F1 | Remove mock-data fallback; show visible "backend unreachable / stale" state | app.js, index.html | 1.5–2h |
| F2 | Relative API URL: `fetch("/metrics")` instead of hardcoded `http://localhost:8080` | app.js       | 0.25h     |
| F3 | `uptime_seconds` rename (lands with A1)                              | app.js             | (in A1)   |
| F4 | Optional: storage card — backend already sends `filesystems[]`, UI ignores it | index.html, app.js | 1h (optional) |

### P3 — server features (optional, recommended for the QEMU demo)
| #  | Item                                                                  | File(s)          | Est          |
|----|-----------------------------------------------------------------------|------------------|--------------|
| S1 | Serve static frontend (index.html/app.js/styles.css) from the same binary → one-command demo on target | http_server.cpp | 2–3h |
| S2 | Port from argv (`--port`), keep 8080 default                          | main.cpp         | 0.25h        |

### P4 — polish / verification
| #  | Item                                                                                                   | File(s)              | Est       |
|----|--------------------------------------------------------------------------------------------------------|----------------------|-----------|
| V1 | Zero-warning pass: add `-Wpedantic -Wshadow` to the existing `-Wall -Wextra` and fix everything        | Makefile + all sources | 1h       |
| V2 | Sanitizer pass: build + run with `-fsanitize=address,undefined`; fix all reports (exercises /proc parsers + threads + server) | Makefile target + fixes | 2–3h |
| V3 | README rewrite (references nonexistent `src/metrics.cpp`, calls uptime "someday", omits server/frontend/sampler) + `.gitignore` cleanup (drop `main`, `a.out`) | README.md, .gitignore | 1–2h |

## 5. Totals

| Scope                                    | Hours        |
|------------------------------------------|--------------|
| P0 bugs (B1–B5)                          | 1.2–1.5h     |
| P1 architecture + robustness (A1–A4)     | 5.25–6.25h   |
| P2 frontend (F1–F3)                      | 1.75–2.25h   |
| P4 polish/verification (V1–V3)           | 4–6h         |
| **Core total**                           | **~12–16h**  |
| Optional: F4 storage card                | +1h          |
| Optional: S1+S2 static serving + port    | +2.25–3.25h  |
| **Everything**                           | **~15–20h**  |

Calendar: full-time ≈ 2–3 days; part-time (~10h/wk) ≈ 2 weeks.

## 6. Execution order

1. B1–B5 — small, independent, do first
2. V1 warnings pass — let the compiler find more before touching architecture
3. A1 sampler wiring + F3 rename — one coherent change
4. A2, A3, A4
5. V2 sanitizers — validate the new architecture under ASan/UBSan
6. F1, F2 (+F4 if desired)
7. S1, S2 (optional)
8. V3 README last, so it describes the final state

## 7. Notes

- After step 5 the codebase is Buildroot-ready: the Makefile already supports
  `CROSS_COMPILE=`, `STATIC=1`, and an `install` target; `-pthread` is set
  (the flag that matters on musl/uClibc).
- Check the `json.hpp` version in its header comment before cross-compiling —
  older vendored versions had C++17 corner-case issues.
- No tests exist; the sanitizer pass is the substitute for now. Unit tests for
  the `/proc` parsers remain the top post-MVP item.
