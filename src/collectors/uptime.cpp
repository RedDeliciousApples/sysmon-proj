#include <chrono>
#include <thread>
#include <iostream>
#include <utility>
#include <cstdio>
#include <cmath>
#include <stdexcept>
#include <stdexcept>

#include <iostream>


long double time_awake(){
    long double time;

    FILE* uptime_stat = fopen("/proc/uptime", "r");

    if (uptime_stat == NULL)                     throw std::runtime_error("time_awake failed to open");
    if (fscanf(uptime_stat, "%Lf", &time) != 1){
        fclose(uptime_stat);
        throw std::runtime_error("time_awake read nothing"); 
    }
    fclose(uptime_stat);
    return time;
}

