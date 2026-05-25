#include <chrono>
#include <thread>
#include <iostream>
#include <utility>
#include <cstdio>
#include <cmath>

#include <iostream>
#include "json.hpp"


using json = nlohmann::json;

using namespace std::this_thread;
using namespace std::chrono;
//maybe unsafe to do this?
using namespace std;

//static is just like "private" in Java

static long double round_to(long double value, int decimals) {
    long double factor = pow(10, decimals);
    return round(value * factor) / factor;
}
//make a get_cpu_usage func that calls this private func
static pair<long double, long double> get_cpu_snapshot()
{
    char cpu[4];
    long double usertime;
    long double nicetime;
    long double systime;
    long double idletime;
    long double iowait, irq, softirq, steal;
    FILE *procstat = fopen("/proc/stat", "r");

    //necessary? look into this
    fscanf(procstat, "%s", cpu);
    
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

long double get_mem_usage(){


    long double total;
    long double memfree;
    long double available;
    //need to check for failure
    FILE *meminfo = fopen("/proc/meminfo", "r");

    fscanf(meminfo, "MemTotal: %Lf kB\n", &total);
    fscanf(meminfo, "MemFree: %Lf kB\n", &memfree);
    fscanf(meminfo, "MemAvailable: %Lf kB\n", &available);

    fclose(meminfo);

    return round_to(100.0L * (1.0L - available / total), 2);

}

nlohmann::json get_metrics_json() {
    nlohmann::json j;

    j["cpu"] = get_cpu_snapshot();
    j["mem"] = get_mem_usage();

    return j;
}