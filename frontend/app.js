const mockMetrics = {
  cpu: 12.4,
  mem: 41.2,
  uptime_seconds: 7980,
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
  document.getElementById("uptime").textContent = formatUptime(metrics.uptime_seconds);
  document.getElementById("loadavg").textContent =
    `${metrics.loadavg["1min"]} / ${metrics.loadavg["5min"]} / ${metrics.loadavg["15min"]}`;
}

renderMetrics(mockMetrics);