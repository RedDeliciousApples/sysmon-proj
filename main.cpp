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


    

    cout << cpu_usage << "%" << endl;

    long double mem_usage = get_mem_usage();



    return 0;
}