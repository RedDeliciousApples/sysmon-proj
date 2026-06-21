#include <iostream>
#include <unistd.h>


#include "collectors/cpu.h"
#include "collectors/mem.h"
#include "collectors/uptime.h"
#include "collectors/loadavg.h"
#include "utils/getjson.h"
#include "json.hpp"

int main(int argc, char** argv)
{
    bool watching = false;
    if (argc > 1)
    {
        if(strcmp(argv[1], "--watch") == 0)
            watching = true;
    }

    if(watching)
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
            std::cout << get_metrics_json().dump(4) << '\n';
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
    std::cout << get_metrics_json().dump(4) << '\n';

    return 0;
}