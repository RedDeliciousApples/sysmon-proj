#include <chrono>
#include <thread>
#include <iostream>
#include <utility>
#include <cstdio>
#include <cmath>
#include <stdexcept>
#include <stdexcept>

#include <iostream>

long double get_uptime()
{
    long double downtime;
    long double totaltime;

    FILE* uptime_stat = fopen("/proc/uptime", "r");


    fscanf(uptime_stat, "%Lf", &downtime);
    fscanf(uptime_stat, "%Lf", &totaltime);

    long double uptime = (totaltime - downtime) / totaltime * 100;
    return uptime;
}

long double time_awake(){
    long double time;

    FILE* uptime_stat = fopen("/proc/uptime", "r");

    if (uptime_stat == NULL)                     throw std::runtime_error("time_awake failed to open");
    if (fscanf(uptime_stat, "%Lf", &time) != 1)  throw std::runtime_error("time_awake read nothing"); 

    fclose(uptime_stat);
    return time;
}

