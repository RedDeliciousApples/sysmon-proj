#include <chrono>
#include <thread>
#include <iostream>
#include <utility>
#include <cstdio>
#include <cmath>
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


    fscanf(uptime_stat, "%Lf", &time);

    
    return time;
}

