#include <iostream>
#include "loadavg.h"
LoadAverage get_load_avg(){

    LoadAverage mystruct;
    mystruct.one_min = mystruct.five_min = mystruct.fifteen_min = 0;

    FILE *load = fopen("/proc/loadavg", "r");
    if (load == nullptr) {
        // handle failure someday...
        //for now just error
        throw std::runtime_error("ERROR! HELP! Failed to open /proc/loadavg");
    }

    fscanf(load, "%Lf", &mystruct.one_min);

    fscanf(load, "%Lf", &mystruct.five_min);

    fscanf(load, "%Lf", &mystruct.fifteen_min);

    fclose(load);
    return mystruct;
}