#pragma once

#include <string>
#include <vector>

struct FilesystemUsage {
    std::string mount_point;
    std::string fs_type;
    unsigned long long total_bytes;
    unsigned long long free_bytes;
    unsigned long long available_bytes;
    long double used_percent;
};

std::vector<FilesystemUsage> get_filesystem_usage();