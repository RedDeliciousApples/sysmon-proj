#include "mem.h"


#include <cstdio>
#include <cmath>
#include <stdexcept>


#include "../utils/math_utils.h"


long double get_mem_usage(){


    long double total;
    long double memfree;
    long double available;
    //need to check for failure
    FILE *meminfo = fopen("/proc/meminfo", "r");

    if (meminfo == nullptr) {
    // handle failure someday...
    //for now just error
        throw std::runtime_error("ERROR! HELP! Failed to open /proc/meminfo");
    }

    fscanf(meminfo, "MemTotal: %Lf kB\n", &total);
    fscanf(meminfo, "MemFree: %Lf kB\n", &memfree);
    fscanf(meminfo, "MemAvailable: %Lf kB\n", &available);

    fclose(meminfo);

    return round_to(100.0L * (1.0L - available / total), 2);

}