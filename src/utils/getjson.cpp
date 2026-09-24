#include "json.hpp"
#include "../collectors/cpu.h"
#include "../collectors/mem.h"
#include "../collectors/uptime.h"
#include "../collectors/loadavg.h"
#include "../collectors/storage.h"


nlohmann::json get_metrics_json()
{
    nlohmann::json j;

    j["cpu"] = get_cpu_usage();
    j["mem"] = get_mem_usage();
    j["awake"] = time_awake();
    LoadAverage load = get_load_avg();

    j["loadavg"] = {
        {"1min", load.one_min},
        {"5min", load.five_min},
        {"15min", load.fifteen_min}
    };

    std::vector<FilesystemUsage> storage_usage = get_filesystem_usage();

    nlohmann::json storage_array = nlohmann::json::array();

    for (const auto& fs : storage_usage) {
        storage_array.push_back({
            {"mount_point", fs.mount_point},
            {"fs_type", fs.fs_type},
            {"total_bytes", fs.total_bytes},
            {"free_bytes", fs.free_bytes},
            {"available_bytes", fs.available_bytes},
            {"used_percent", static_cast<double>(fs.used_percent)}
        });
    }

    j["storage"] = {
        {"filesystems", storage_array}
    };

    
    return j;
}
