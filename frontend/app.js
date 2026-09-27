function formatUptime(seconds) {
  const hours = Math.floor(seconds / 3600);
  const minutes = Math.floor((seconds % 3600) / 60);
  return `${hours}h ${minutes}m`;
}

const REQUEST_TIMEOUT_MS = 3000;

function setStatus(message, stale) {
  const status = document.getElementById("status");
  status.textContent = message;
  status.classList.toggle("stale", stale);
}

function formatMetric(value, suffix = "") {
  return value == null ? "--" : `${value}${suffix}`;
}

function renderMetrics(metrics) {
  if (!metrics || typeof metrics !== "object") {
    throw new Error("Invalid metrics response");
  }

  document.getElementById("cpu").textContent = formatMetric(metrics.cpu_percent, "%");
  document.getElementById("mem").textContent = formatMetric(metrics.memory_used_percent, "%");
  const seconds = metrics.uptime_seconds;
  document.getElementById("uptime").textContent =
    seconds == null ? "--" : formatUptime(seconds);

  const loadavg = metrics.load_average;
  document.getElementById("loadavg").textContent = loadavg
    ? formatMetric(loadavg.one_min) + " / " + formatMetric(loadavg.five_min) + " / " + formatMetric(loadavg.fifteen_min)
    : "--";
  setStatus("Backend connected", false);
}

async function fetchMetricsOnce() {
  const controller = new AbortController();
  const timeoutId = setTimeout(() => controller.abort(), REQUEST_TIMEOUT_MS);

  try {
    const res = await fetch("/metrics", { signal: controller.signal });
    if (res.status === 503) {
      setStatus("Collecting first sample…", true);
      return;
    }
    if (!res.ok) throw new Error(`HTTP ${res.status}`);
    const json = await res.json();
    renderMetrics(json);
    return;
  } catch (err) {
    console.warn("Failed to fetch metrics:", err);
    setStatus("Backend unavailable", true);
  } finally {
    clearTimeout(timeoutId);
  }
}

// initial render and polling
fetchMetricsOnce();
setInterval(fetchMetricsOnce, 2000);