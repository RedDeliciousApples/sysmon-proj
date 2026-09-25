#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

struct LoadAverage {
    double one_min;
    double five_min;
    double fifteen_min;
};

struct FilesystemUsage {
    std::string mount_point;
    std::string fs_type;
    std::uint64_t total_bytes;
    std::uint64_t free_bytes;
    std::uint64_t available_bytes;
    double used_percent;
};

struct MetricsSnapshot {

    std::int64_t collected_at_unix_seconds;


    std::optional<double> cpu_percent;

    double memory_used_percent;
    double uptime_seconds;
    LoadAverage load_average;
    std::vector<FilesystemUsage> filesystems;

  
    std::uint64_t sequence;
};