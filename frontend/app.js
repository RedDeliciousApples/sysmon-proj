const mockMetrics = {
  cpu: 12.4,
  mem: 41.2,
  awake: 7980,
  loadavg: {
    "1min": 0.52,
    "5min": 0.48,
    "15min": 0.45
  }
};

function formatUptime(seconds) {
  const hours = Math.floor(seconds / 3600);
  const minutes = Math.floor((seconds % 3600) / 60);
  return `${hours}h ${minutes}m`;
}

function renderMetrics(metrics) {
  document.getElementById("cpu").textContent = `${metrics.cpu}%`;
  document.getElementById("mem").textContent = `${metrics.mem}%`;
  // server returns `awake` (seconds), fallback keeps compatibility
  const seconds = metrics.awake ?? metrics.uptime_seconds ?? 0;
  document.getElementById("uptime").textContent = formatUptime(seconds);
  document.getElementById("loadavg").textContent =
    `${metrics.loadavg["1min"]} / ${metrics.loadavg["5min"]} / ${metrics.loadavg["15min"]}`;
}

async function fetchMetricsOnce() {
  try {
    const res = await fetch("http://localhost:8080/metrics");
    if (!res.ok) throw new Error(`HTTP ${res.status}`);
    const json = await res.json();
    renderMetrics(json);
    return;
  } catch (err) {
    // network failed or server not available; keep using mock data
    console.warn("Failed to fetch metrics, using mock data:", err);
    renderMetrics(mockMetrics);
  }
}

// initial render and polling
fetchMetricsOnce();
setInterval(fetchMetricsOnce, 2000);