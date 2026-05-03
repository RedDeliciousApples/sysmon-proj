#include <chrono>
#include <thread>
#include <iostream>
#include <utility>
#include <cstdio>

using namespace std::this_thread;
using namespace std::chrono;
using namespace std;
<<<<<<< HEAD
long double get_snapshot_total(char *cpu, long double &usertime, long double &nicetime, long double &systime, long double &idletime, FILE *procstat){
    
=======

pair<long double, long double> get_snapshot(char *cpu, long double &usertime, long double &nicetime, long double &systime, long double &idletime)
{
    FILE *procstat = fopen("/proc/stat", "r");

>>>>>>> 77b9783 (pair)
    fscanf(procstat, "%s", cpu);
    fscanf(procstat, "%Lf", &usertime);
    fscanf(procstat, "%Lf", &nicetime);
    fscanf(procstat, "%Lf", &systime);
    fscanf(procstat, "%Lf", &idletime);

    fclose(procstat);

    long double total = usertime + nicetime + systime + idletime;
<<<<<<< HEAD
    
    return total;
}
long double get_snapshot_idle(char *cpu, long double &usertime, long double &nicetime, long double &systime, long double &idletime, FILE* procstat){
    
    fscanf(procstat, "%s", cpu);
    fscanf(procstat, "%Lf", &usertime);
    fscanf(procstat, "%Lf", &nicetime);
    fscanf(procstat, "%Lf", &systime);
    fscanf(procstat, "%Lf", &idletime);
    
    return idletime;
=======
    return {idletime, total};
>>>>>>> 77b9783 (pair)
}

int main() {
    char cpu[4];
    long double usertime;
    long double nicetime;
    long double systime;
    long double idletime;

<<<<<<< HEAD
    
    // for (int i =0; i < 20; i ++)
    // {
    //     cout << " test " << i << endl;
    //     //sleep_for(nanoseconds(10));
    //     sleep_for(seconds(1));
    // }
    long double t1 = get_snapshot_total(cpu, usertime, nicetime, systime, idletime, procstat);
     long double idle_t1 = get_snapshot_idle(cpu, usertime, nicetime, systime, idletime, procstat);
    sleep_for(seconds(1));
    long double t2 = get_snapshot_total(cpu, usertime, nicetime, systime, idletime, procstat);
    long double idle_t2 = get_snapshot_idle(cpu, usertime, nicetime, systime, idletime, procstat);
    long double delta_time = t2 - t1;
    long double delta_idle = idle_t2 - idle_t1;

    long double cpu_usage = 100 *(1- (delta_idle/delta_time));

    cout << cpu_usage << "%" << endl;
    
   
    fclose(procstat);
    
=======
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

>>>>>>> 77b9783 (pair)
    return 0;
}