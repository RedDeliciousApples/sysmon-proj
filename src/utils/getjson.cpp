#include "json.hpp"
#include "../collectors/cpu.h"
#include "../collectors/mem.h"
#include "../collectors/uptime.h"
#include "../collectors/loadavg.h"


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
    
    return j;
}
