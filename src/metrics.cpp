#include <chrono>
#include <thread>
#include <iostream>
#include <utility>
#include <cstdio>
#include <cmath>
#include <stdexcept>

#include <iostream>
#include "json.hpp"


using json = nlohmann::json;

using namespace std::this_thread;
using namespace std::chrono;
//maybe unsafe to do this?
//using namespace std;

//static is just like "private" in Java

static long double round_to(long double value, int decimals) {
    long double factor = pow(10, decimals);
    return round(value * factor) / factor;
}


nlohmann::json get_metrics_json() {
    nlohmann::json j;

    j["cpu"] = get_cpu_usage();
    j["mem"] = get_mem_usage();

    return j;
}