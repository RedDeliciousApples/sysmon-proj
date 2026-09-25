#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include "loadavg.h"
#include "storage.h"


struct MetricsSnapshot {

    std::int64_t collected_at_unix_seconds;


    std::optional<double> cpu_percent;

    double memory_used_percent;
    double uptime_seconds;
    LoadAverage load_average;
    std::vector<FilesystemUsage> filesystems;

  
    std::uint64_t sequence;
};