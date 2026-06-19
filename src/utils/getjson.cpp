#include "json.hpp"
#include "../collectors/cpu.h"
#include "../collectors/mem.h"
#include "../collectors/uptime.h"


nlohmann::json get_metrics_json()
{
    nlohmann::json j;

    j["cpu"] = get_cpu_usage();
    j["mem"] = get_mem_usage();
    j["awake"] = time_awake();
    return j;
}
