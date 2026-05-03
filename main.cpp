#include <chrono>
#include <thread>
#include <iostream>
#include <utility>
#include <cstdio>

using namespace std::this_thread;
using namespace std::chrono;
using namespace std;

pair<long double, long double> get_snapshot(char *cpu, long double &usertime, long double &nicetime, long double &systime, long double &idletime)
{
    FILE *procstat = fopen("/proc/stat", "r");

    fscanf(procstat, "%s", cpu);
    fscanf(procstat, "%Lf", &usertime);
    fscanf(procstat, "%Lf", &nicetime);
    fscanf(procstat, "%Lf", &systime);
    fscanf(procstat, "%Lf", &idletime);

    fclose(procstat);

    long double total = usertime + nicetime + systime + idletime;
    return {idletime, total};
}

int main() {
    char cpu[4];
    long double usertime;
    long double nicetime;
    long double systime;
    long double idletime;

    pair<long double, long double> snapshot1 = get_snapshot(cpu, usertime, nicetime, systime, idletime);

    long double idle_t1 = snapshot1.first;
    long double t1 = snapshot1.second;

    sleep_for(seconds(1));

    pair<long double, long double> snapshot2 = get_snapshot(cpu, usertime, nicetime, systime, idletime);

    long double idle_t2 = snapshot2.first;
    long double t2 = snapshot2.second;

    long double delta_time = t2 - t1;
    long double delta_idle = idle_t2 - idle_t1;

    long double cpu_usage = 100 * (1 - (delta_idle / delta_time));

    cout << cpu_usage << "%" << endl;

    return 0;
}