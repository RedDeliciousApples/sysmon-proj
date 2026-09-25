#include "mem.h"


#include <cstdio>
#include <cmath>
#include <stdexcept>


#include "../utils/math_utils.h"


long double get_mem_usage(){


    long double total = 0.0L;
    long double memfree = 0.0L;
    long double available = 0.0L;

    FILE *meminfo = fopen("/proc/meminfo", "r");

    if (meminfo == nullptr) {
        throw std::runtime_error("ERROR! HELP! Failed to open /proc/meminfo");
    }

    fscanf(meminfo, "MemTotal: %Lf kB\n", &total);
    fscanf(meminfo, "MemFree: %Lf kB\n", &memfree);
    fscanf(meminfo, "MemAvailable: %Lf kB\n", &available);

    fclose(meminfo);
    if (total <= 0.0L || available < 0.0L) {
        throw std::runtime_error("ERROR! HELP! Total memory or available memory is 0 or negative");
    }
    return round_to(100.0L * (1.0L - available / total), 2);

}