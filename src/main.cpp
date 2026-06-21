#include <iostream>

#include "collectors/cpu.h"
#include "collectors/mem.h"
#include "collectors/uptime.h"
#include "collectors/loadavg.h"
#include "utils/getjson.h"
#include "json.hpp"

int main()
{
    std::cout << "CPU usage: "
              << get_cpu_usage()
              << "%\n";

    std::cout << "Memory usage: "
              << get_mem_usage()
              << "%\n";
    std::cout << "uptime in seconds:" << time_awake() << " seconds \n";
    std::cout << "\nJSON:\n";
    std::cout << get_metrics_json().dump(4) << '\n';

    return 0;
}