#include <chrono>
#include <thread>
#include <iostream>
#include <utility>
#include <cstdio>

#include <iostream>
#include "json.hpp"

using json = nlohmann::json;

int main() {
    json j;
    j["cpu"] = 12.4;
    j["mem"] = 58.2;

    std::cout << j.dump() << std::endl;
}

using json = nlohmann::json;

using namespace std::this_thread;
using namespace std::chrono;
using namespace std;

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
    return {idletime, total};
}

long double get_mem_usage(){

    char memtotalstr[10];
    char kb[3];
    char freestr[9];
    long double total;
    long double memfree;
    FILE *meminfo = fopen("/proc/meminfo", "r");

    fscanf(meminfo, "MemTotal: %Lf kB\n", &total);
    fscanf(meminfo, "MemFree: %Lf kB\n", &memfree);

    fclose(meminfo);

    return memfree;

}

// int main() {
//     char cpu[4];
//     long double usertime;
//     long double nicetime;
//     long double systime;
//     long double idletime;
//     long double iowait, irq, softirq, steal;

//     pair<long double, long double> snapshot1 = get_snapshot(cpu, usertime, nicetime, systime, idletime, iowait, irq, softirq, steal);

//     long double idle_t1 = snapshot1.first;
//     long double t1 = snapshot1.second;

//     sleep_for(seconds(1));

//     pair<long double, long double> snapshot2 = get_snapshot(cpu, usertime, nicetime, systime, idletime, iowait, irq, softirq, steal);

//     long double idle_t2 = snapshot2.first;
//     long double t2 = snapshot2.second;

//     long double delta_time = t2 - t1;
//     long double delta_idle = idle_t2 - idle_t1;

//     long double cpu_usage = 100 * (1 - (delta_idle / delta_time));

//     cout << cpu_usage << "%" << endl;

//     cout << get_mem_usage() << endl;

//     return 0;
// }