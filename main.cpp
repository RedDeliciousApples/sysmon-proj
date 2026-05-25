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



    return 0;
}