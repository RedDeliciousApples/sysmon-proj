#include <chrono>
#include <thread>
#include <iostream>
#include <utility>
#include <cstdio>
#include <cmath>
#include <stdexcept>

#include <iostream>
#include "json.hpp"
#include "cpu.h"

#include "../utils/math_utils.h"


using json = nlohmann::json;

using namespace std::this_thread;
using namespace std::chrono;
//make a get_cpu_usage func that calls this private func
static std::pair<long double, long double> get_cpu_snapshot()
{
    char cpu[4];
    long double usertime;
    long double nicetime;
    long double systime;
    long double idletime;
    long double iowait, irq, softirq, steal;
    FILE *procstat = fopen("/proc/stat", "r");

    if (procstat == nullptr) {
        // handle failure someday...
        //for now just error
        throw std::runtime_error("ERROR! HELP! Failed to open /proc/stat");
    }

    //necessary? look into this
    fscanf(procstat, "%3s", cpu);
    
    fscanf(procstat, "%Lf", &usertime);
    fscanf(procstat, "%Lf", &nicetime);
    fscanf(procstat, "%Lf", &systime);
    fscanf(procstat, "%Lf", &idletime);
    fscanf(procstat, "%Lf", &iowait);
    fscanf(procstat, "%Lf", &irq);
    fscanf(procstat, "%Lf", &softirq);
    fscanf(procstat, "%Lf", &steal);

    fclose(procstat);

    long double total = usertime + nicetime + systime + idletime + iowait + irq + softirq + steal;
    return {idletime + iowait, total};
}



long double get_cpu_usage(){
    std::pair<long double, long double> snapshot1 = get_cpu_snapshot();

    long double idle_t1 = snapshot1.first;
    long double t1 = snapshot1.second;

    sleep_for(seconds(1));

    std::pair<long double, long double> snapshot2 = get_cpu_snapshot();

    long double idle_t2 = snapshot2.first;
    long double t2 = snapshot2.second;

    long double delta_time = t2 - t1;
    long double delta_idle = idle_t2 - idle_t1;

    if (delta_time <= 0) {
        throw std::runtime_error("Invalid CPU snapshot: total CPU time did not increase");
    }
    
    long double cpu_usage = round_to(100 * (1 - (delta_idle / delta_time)), 2);
    return cpu_usage;
}