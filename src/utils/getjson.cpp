#include "json.hpp"
#include "../collectors/metrics_snapshot.h"

nlohmann::json get_metrics_json_mine(const MetricsSnapshot& snapshot)
{
    nlohmann::json json;

    if (snapshot.cpu_percent) {
        json["cpu_percent"] = *snapshot.cpu_percent;
    } else {
        json["cpu_percent"] = nullptr;
    }

    json["memory_used_percent"] = snapshot.memory_used_percent;
    json["uptime_seconds"] = snapshot.uptime_seconds;
    json["collected_at_unix_seconds"] =
        snapshot.collected_at_unix_seconds;
    json["sequence"] = snapshot.sequence;

    json["load_average"] = {
        {"one_min", snapshot.load_average.one_min},
        {"five_min", snapshot.load_average.five_min},
        {"fifteen_min", snapshot.load_average.fifteen_min}
    };

    
    return json;
}
