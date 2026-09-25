# sysmon — revised code-only estimate (post review)

Scope: everything needed to make this codebase "near perfect" before Buildroot
work starts. This revised estimate reflects the current repo state, not the
idealized version described in the original draft.

## 1. Files reviewed

Reviewed in the workspace:
- `Makefile`
- `README.md`
- `src/main.cpp`
- `src/utils/getjson.{h,cpp}`
- `src/utils/math_utils.{h,cpp}`
- `src/collectors/{cpu,mem,loadavg,uptime,storage}.{h,cpp}`
- `src/collectors/metrics_sampler.{h,cpp}`
- `src/collectors/metrics_snapshot.h`
- `src/collectors/sampler_test.cpp`
- `src/server/http_server.{h,cpp}`
- `frontend/{index.html,app.js,styles.css}`

Notable current findings:
- The project does not build cleanly as-is: `make clean && make -j4` fails due a
  duplicate `main` from `src/collectors/sampler_test.cpp`.
- The original estimate assumed the repo had no tests; this repo does contain a
  test-style file, but it is not wired into the build correctly.
- Several `/proc` parser paths are still unguarded and currently rely on implicit
  success from `fscanf`.

## 2. Confirmed issues (revised P0 — fix first)

| #  | File                       | Issue                                                                                 | Est  |
|----|----------------------------|---------------------------------------------------------------------------------------|------|
| B0 | `src/collectors/sampler_test.cpp` | Separate `main()` causes duplicate-definition link failure with the app binary | 0.25h |
| B1 | `src/collectors/mem.cpp` | `total`/`available` are effectively uninitialized on parse failure; UB/NaN risk | 0.5h |
| B2 | `src/collectors/uptime.cpp` | `get_uptime()` is dead code, has no `fopen` check, no `fclose`, and bad math | 0.5h |
| B3 | `src/main.cpp` | `strcmp` without `<cstring>`; relies on incidental includes | 0.1h |
| B4 | `src/server/http_server.cpp` | `send()` lacks `MSG_NOSIGNAL` handling; disconnects can kill the server | 0.5h |
| B5 | `frontend/styles.css` | `.body` selector mismatches `<body>` and never applies | 0.1h |
| B6 | `src/collectors/loadavg.cpp` | parse results are not checked; silent zeroing on failure | 0.5h |

The real project-state gap is not just the original five bugs; it also includes
basic build hygiene and a one-off test harness that currently blocks the normal build.

## 3. Architecture assessment

The original architecture assessment is broadly correct, but it is still optimistic.
The codebase is not yet in the state where the sampler can be safely wired into the
server without a build and warnings pass first.

Current state:
- `MetricsSampler` exists, but it is not integrated into runtime server flow.
- `main.cpp` still calls the collectors directly in `--watch` mode and serves via
  a blocking request loop.
- `/metrics` generation in `get_metrics_json()` is synchronous and not snapshot-based.
- The server is single-threaded and blocks on a single accept cycle.

Target architecture still makes sense:
- `main.cpp`: create/start `MetricsSampler`, pass it to the HTTP server.
- `http_server.cpp`: `GET /metrics` serves `sampler.latest()` instead of recomputing values per request.
- `getjson.cpp`: becomes a serializer for a snapshot (and must handle the first sample where CPU is unset).
- `--watch` keeps direct collector behavior for CLI output.

## 4. Revised punch list

### P1 — build stabilization + robustness
| #  | Item                                                                 | File(s) | Est |
|----|-----------------------------------------------------------------------|---------|-----|
| A0 | Fix duplicate `main` problem from the sampler test harness | `src/collectors/sampler_test.cpp`, build config | 0.5h |
| A1 | Wire sampler into server and remove per-request collector calls | `src/main.cpp`, `src/server/http_server.{h,cpp}`, `src/utils/getjson.cpp` | 4–5h |
| A2 | Make `stop()` prompt and safe under a worker loop (`condition_variable` or equivalent) | `src/collectors/metrics_sampler.{h,cpp}` | 1–1.5h |
| A3 | Check `fscanf` return values in `/proc` collectors | `src/collectors/{mem,loadavg,uptime}.cpp` | 1h |
| A4 | Document the error policy: collectors throw; sampler catches/logs and keeps last snapshot | relevant collector/sampler comments | 0.25h |

### P2 — frontend honesty
| #  | Item | File(s) | Est |
|----|-------|---------|-----|
| F1 | Remove mock-data fallback; show a visible stale/unreachable state | `frontend/app.js`, `frontend/index.html` | 1.5–2h |
| F2 | Switch to relative fetch URL (`/metrics`) | `frontend/app.js` | 0.25h |
| F3 | Rename `awake` to `uptime_seconds` and align frontend labels | `frontend/app.js`, `src/utils/getjson.cpp` | 0.5h |
| F4 | Optional: add storage card for `filesystems[]` | `frontend/index.html`, `frontend/app.js` | 1h |

### P3 — server features (optional)
| #  | Item | File(s) | Est |
|----|-------|---------|-----|
| S1 | Serve frontend assets from the same binary | `src/server/http_server.cpp` | 2–3h |
| S2 | Accept `--port` and default to 8080 | `src/main.cpp` | 0.25h |

### P4 — polish / verification
| #  | Item | File(s) | Est |
|----|-------|---------|-----|
| V1 | Warning pass with `-Wpedantic -Wshadow`; fix all warnings | `Makefile`, all sources | 1–2h |
| V2 | ASan/UBSan validation pass | `Makefile`, all sources | 2.5–4h |
| V3 | README and ignore cleanup | `README.md`, `.gitignore` | 1–2h |

## 5. Revised totals

| Scope | Hours |
|-------|-------|
| P0 bugs (B0–B6) | 2.25–3.25h |
| P1 architecture + robustness (A0–A4) | 6.75–8.25h |
| P2 frontend (F1–F3) | 2.5–3.75h |
| P4 polish/verification (V1–V3) | 4.5–8h |
| **Core total** | **~16–23h** |
| Optional: F4 storage card | +1h |
| Optional: S1+S2 static serving + port | +2.5–3.5h |
| **Everything** | **~19–27h** |

That is still within the general range described in the original estimate, but it is
now more realistic for the current state of the repo. The main difference is that
there is a real build-blocker and several robustness gaps before the architecture work
can be considered cleanly implementable.

## 6. Revised execution order

1. B0–B6 — eliminate blockers and obvious UB paths first
2. V1 warnings pass — compiler feedback will reveal additional cleanup
3. A1 sampler wiring + F3 rename — one coherent change
4. A2, A4
5. V2 sanitizers — verify thread and `/proc` behavior under ASan/UBSan
6. F1, F2, F4 if desired
7. S1, S2 (optional)
8. V3 README and ignore cleanup last

## 7. Notes

- The project is not yet in a stable build state; the duplicate `main` in the test file
  is a real regression blocker and should be treated as such.
- The original estimate was directionally sound but undercounted the amount of clean-up
  required before architecture work becomes safe.
- After the warnings/sanitizer passes, the repo should be much closer to Buildroot-ready,
  but only after the test harness and the currently blocking runtime assumptions are fixed.
