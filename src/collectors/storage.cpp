#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <sys/statvfs.h>

#include "storage.h"
#include "../utils/math_utils.h"

static const std::vector<std::string> SKIP_FILESYSTEMS = {
    "proc", "sysfs", "devtmpfs", "tmpfs", "devpts", 
    "cgroup", "cgroup2", "overlay", "squashfs"
};

//unused for now
// static const std::vector<std::string> ALLOW_FILESYSTEMS = {
//     "ext4", "xfs", "btrfs", "zfs"
// };

static bool is_skip_filesystem(const std::string& fs_type) {
    for (const auto& type : SKIP_FILESYSTEMS) {
        if (type == fs_type) {
            return true;
        }
    }

    return false; 
}

std::vector<FilesystemUsage> get_filesystem_usage() {
    std::vector<FilesystemUsage> usage_list;
    
    // Open /proc/mounts for reading
    std::ifstream mount_file("/proc/mounts");
    if (!mount_file.is_open()) {
        return usage_list;
    }

    std::string line;
    while (std::getline(mount_file, line)) {
        // skip empty lines
        if (line.empty()) continue;


        // Format: device mountpoint fstype options ...
        // Using a stringstream to tokenize by whitespace
        std::istringstream ss(line);
        std::string device, mount_point, fs_type;

        if (!(ss >> device >> mount_point >> fs_type)) {
            continue; // Not enough fields in line
        }

        // Filter: Skip weird virtual filesystems
        if (is_skip_filesystem(fs_type)) {
            continue;
        }

        // Call statvfs
        struct statvfs stats{};
        if (statvfs(mount_point.c_str(), &stats) != 0) {
            continue; // statvfs failed, skip this mount
        }

        // Calculate bytes
        unsigned long long f_blocks = static_cast<unsigned long long>(stats.f_blocks);
        unsigned long long f_frsize = static_cast<unsigned long long>(stats.f_frsize);
        unsigned long long f_bfree = static_cast<unsigned long long>(stats.f_bfree);
        unsigned long long f_bavail = static_cast<unsigned long long>(stats.f_bavail);


        unsigned long long total_bytes = f_blocks * f_frsize;
        unsigned long long free_bytes = f_bfree * f_frsize;
        unsigned long long available_bytes = f_bavail * f_frsize;

        FilesystemUsage usage;
        usage.fs_type = fs_type;
        usage.mount_point = mount_point;
        usage.total_bytes = total_bytes;
        usage.free_bytes = free_bytes;
        usage.available_bytes = available_bytes;


        if (total_bytes > 0) {
            long double avail_value = static_cast<long double>(available_bytes);
            long double total_value = static_cast<long double>(total_bytes);
            usage.used_percent = round_to(100.0L * (1.0L - (avail_value / total_value)), 2);
        } else {
            usage.used_percent = 0.0; // Avoid NaN/Inf if total is 0
        }

        usage_list.push_back(usage);
    }

    mount_file.close();
    return usage_list;
}
