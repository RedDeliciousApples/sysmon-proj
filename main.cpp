#include <chrono>
#include <thread>
#include <iostream>
using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono; // nanoseconds, system_clock, seconds
using namespace std;
long double get_snapshot_total(char *cpu, long double &usertime, long double &nicetime, long double &systime, long double &idletime, FILE *procstat){
    
    fscanf(procstat, "%s", cpu);
    fscanf(procstat, "%Lf", &usertime);
    fscanf(procstat, "%Lf", &nicetime);
    fscanf(procstat, "%Lf", &systime);
    fscanf(procstat, "%Lf", &idletime);
    long double total = usertime + nicetime + systime + idletime;
    
    return total;
}
long double get_snapshot_idle(char *cpu, long double &usertime, long double &nicetime, long double &systime, long double &idletime, FILE* procstat){
    
    fscanf(procstat, "%s", cpu);
    fscanf(procstat, "%Lf", &usertime);
    fscanf(procstat, "%Lf", &nicetime);
    fscanf(procstat, "%Lf", &systime);
    fscanf(procstat, "%Lf", &idletime);
    
    return idletime;
}
int main() {
    FILE *procstat = fopen("/proc/stat", "r");
    char cpu[4];
    long double usertime;
    long double nicetime;
    long double systime;
    long double idletime;

    
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
    
    return 0;
}