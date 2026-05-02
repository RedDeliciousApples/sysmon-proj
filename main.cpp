#include <chrono>
#include <thread>
#include <iostream>
using namespace std::this_thread; // sleep_for, sleep_until
using namespace std::chrono; // nanoseconds, system_clock, seconds
using namespace std;
int main() {
    FILE *procstat = fopen("/proc/stat", "r");
    char cpu[4];
    long double usertime;
    long double nicetime;
    long double systime;
    long double idletime;
    fscanf(procstat, "%s", cpu);
    fscanf(procstat, "%Lf", &usertime);
    fscanf(procstat, "%Lf", &nicetime);
    fscanf(procstat, "%Lf", &systime);
    fscanf(procstat, "%Lf", &idletime);
    cout << "cpu is: " << cpu << " and user time is: " << usertime << " and nice time is: " << nicetime  << " sys time is" << systime << " idle is" << idletime << endl;
    // for (int i =0; i < 20; i ++)
    // {
    //     cout << " test " << i << endl;
    //     //sleep_for(nanoseconds(10));
    //     sleep_for(seconds(1));
    // }
    return 0;
}