#include <iostream>
#include <unistd.h>
#include <cstring>

#include "collectors/cpu.h"
#include "collectors/mem.h"
#include "collectors/uptime.h"
#include "collectors/loadavg.h"
#include "utils/getjson.h"
#include "json.hpp"
#include "server/http_server.h"
#include "collectors/metrics_sampler.h"

int main(int argc, char** argv)
{
    bool watching = false;
    bool serving = false;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--watch") == 0) watching = true;
        if (strcmp(argv[i], "--serve") == 0) serving = true;
    }

    if (serving) {
        MetricsSampler sampler(std::chrono::seconds(1));
        sampler.start();

        run_server(8080, sampler);
        return 0;
    }

    if (watching)
    {
        while(true)
        {
            std::cout << "CPU usage: "
                << get_cpu_usage()
                << "%\n";

            std::cout << "Memory usage: "
                << get_mem_usage()
                << "%\n";
            std::cout << "uptime in seconds:" << time_awake() << " seconds \n";
            std::cout << "\nJSON:\n";
            std::cout << get_metrics_json_mine(MetricsSnapshot()).dump(4) << '\n';
            sleep(1);                
        }
    }
    std::cout << "CPU usage: "
              << get_cpu_usage()
              << "%\n";

    std::cout << "Memory usage: "
              << get_mem_usage()
              << "%\n";
    std::cout << "uptime in seconds:" << time_awake() << " seconds \n";
    std::cout << "\nJSON:\n";
    std::cout << get_metrics_json_mine(MetricsSnapshot()).dump(4) << '\n';

    return 0;
}