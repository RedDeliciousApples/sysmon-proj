import { expect, test } from "bun:test";
import { readFileSync } from "node:fs";

const appSource = readFileSync(new URL("./app.js", import.meta.url), "utf8")
  // omit polling so tests can call one request at a time
  .replace(/\/\/ initial render and polling[\s\S]*$/, "");

function createHarness(fetchImpl) {
  const elements = new Map();
  let timeoutCallback;

  function getElementById(id) {
    if (!elements.has(id)) {
      elements.set(id, {
        textContent: "",
        classList: { toggle() {} }
      });
    }
    return elements.get(id);
  }

  const functions = new Function(
    "document",
    "fetch",
    "setTimeout",
    "clearTimeout",
    `${appSource}
return { fetchMetricsOnce, renderMetrics };`
  )(
    { getElementById },
    fetchImpl,
    (callback) => {
      timeoutCallback = callback;
      return 1;
    },
    () => {}
  );

  return {
    ...functions,
    elements,
    triggerTimeout() {
      timeoutCallback();
    }
  };
}

const metricsFixture = {
  // matches the JSON emitted by get_metrics_json_mine()
  cpu_percent: 12.4,
  memory_used_percent: 41.2,
  uptime_seconds: 7980,
  load_average: {
    one_min: 0.52,
    five_min: 0.48,
    fifteen_min: 0.45
  }
};

test("renders the server metrics fixture", async () => {
  const harness = createHarness(async () => ({ ok: true, status: 200, json: async () => metricsFixture }));

  await harness.fetchMetricsOnce();

  expect(harness.elements.get("cpu").textContent).toBe("12.4%");
  expect(harness.elements.get("mem").textContent).toBe("41.2%");
  expect(harness.elements.get("uptime").textContent).toBe("2h 13m");
  expect(harness.elements.get("loadavg").textContent).toBe("0.52 / 0.48 / 0.45");
  expect(harness.elements.get("status").textContent).toBe("Backend connected");
});

test("shows the warming-up status for HTTP 503", async () => {
  const harness = createHarness(async () => ({ ok: false, status: 503 }));

  await harness.fetchMetricsOnce();

  expect(harness.elements.get("status").textContent).toBe("Collecting first sample…");
});

test("shows unavailable status when the backend is offline", async () => {
  const harness = createHarness(async () => {
    throw new Error("offline");
  });

  await harness.fetchMetricsOnce();

  expect(harness.elements.get("status").textContent).toBe("Backend unavailable");
});

test("shows unavailable status when the request times out", async () => {
  const harness = createHarness((_url, { signal }) => new Promise((_resolve, reject) => {
    signal.addEventListener("abort", () => reject(new Error("aborted")));
  }));
  const request = harness.fetchMetricsOnce();

  // trigger the fake timer instead of waiting three seconds
  harness.triggerTimeout();
  await request;

  expect(harness.elements.get("status").textContent).toBe("Backend unavailable");
});

test("renders missing and null CPU values as placeholders", () => {
  const harness = createHarness(async () => ({ ok: true, status: 200, json: async () => ({}) }));

  harness.renderMetrics({ ...metricsFixture, cpu_percent: null });
  expect(harness.elements.get("cpu").textContent).toBe("--");

  const { cpu_percent: _ignored, ...missingCpu } = metricsFixture;
  harness.renderMetrics(missingCpu);
  expect(harness.elements.get("cpu").textContent).toBe("--");
});
