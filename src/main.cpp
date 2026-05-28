#include <iostream>
#include "metrics.hpp"




int main() {
        ///////////////////////////
        //QUICK TESTS
        ///////////////////////////

        std::cout << "CPU usage: " << get_cpu_usage() << "%\n";
        std::cout << "Memory usage: " << get_mem_usage() << "%\n";

        std::cout << "JSON:\n";
        //add .dump(4) for prettier json maybe?
        std::cout << get_metrics_json() << std::endl;



    return 0;
}