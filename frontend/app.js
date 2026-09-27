function formatUptime(seconds) {
  const hours = Math.floor(seconds / 3600);
  const minutes = Math.floor((seconds % 3600) / 60);
  return `${hours}h ${minutes}m`;
}

function setStatus(message, stale) {
  const status = document.getElementById("status");
  status.textContent = message;
  status.classList.toggle("stale", stale);
}

function renderMetrics(metrics) {
  document.getElementById("cpu").textContent = `${metrics.cpu}%`;
  document.getElementById("mem").textContent = `${metrics.mem}%`;
  // server returns `awake` (seconds), fallback keeps compatibility
  const seconds = metrics.awake ?? metrics.uptime_seconds ?? 0;
  document.getElementById("uptime").textContent = formatUptime(seconds);
  document.getElementById("loadavg").textContent =
    `${metrics.loadavg["1min"]} / ${metrics.loadavg["5min"]} / ${metrics.loadavg["15min"]}`;
  setStatus("Backend connected", false);
}

async function fetchMetricsOnce() {
  try {
    const res = await fetch("/metrics");
    if (!res.ok) throw new Error(`HTTP ${res.status}`);
    const json = await res.json();
    renderMetrics(json);
    return;
  } catch (err) {
    console.warn("Failed to fetch metrics:", err);
    setStatus("Backend unavailable", true);
  }
}

// initial render and polling
fetchMetricsOnce();
setInterval(fetchMetricsOnce, 2000);