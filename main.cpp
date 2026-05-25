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
using namespace std;

long double round_to(long double value, int decimals) {
    long double factor = pow(10, decimals);
    return round(value * factor) / factor;
}

pair<long double, long double> get_snapshot()
{
    char cpu[4];
    long double usertime;
    long double nicetime;
    long double systime;
    long double idletime;
    long double iowait, irq, softirq, steal;
    FILE *procstat = fopen("/proc/stat", "r");

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
    FILE *meminfo = fopen("/proc/meminfo", "r");

    fscanf(meminfo, "MemTotal: %Lf kB\n", &total);
    fscanf(meminfo, "MemFree: %Lf kB\n", &memfree);
    fscanf(meminfo, "MemAvailable: %Lf kB\n", &available);

    fclose(meminfo);

    return round_to(100.0L * (1.0L - available / total), 2);

}

int main() {


    pair<long double, long double> snapshot1 = get_snapshot();

    long double idle_t1 = snapshot1.first;
    long double t1 = snapshot1.second;

    sleep_for(seconds(1));

    pair<long double, long double> snapshot2 = get_snapshot();

    long double idle_t2 = snapshot2.first;
    long double t2 = snapshot2.second;

    long double delta_time = t2 - t1;
    long double delta_idle = idle_t2 - idle_t1;

    long double cpu_usage = round_to(100 * (1 - (delta_idle / delta_time)), 2);

    cout << cpu_usage << "%" << endl;

    long double mem_usage = get_mem_usage();

    json j;
    j["cpu"] = cpu_usage;
    j["mem"] = mem_usage;

    std::cout << j.dump() << std::endl;

    return 0;
}