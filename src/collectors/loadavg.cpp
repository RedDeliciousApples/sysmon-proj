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

    if (fscanf(load, "%Lf", &mystruct.one_min)!= 1)
    {
        fclose(load);
        throw std::runtime_error("could not read load avg 1 min"); 
    }

    if (fscanf(load, "%Lf", &mystruct.five_min) != 1)
    {
        fclose(load);
        throw std::runtime_error("could not read load avg 5 min");
    }

    if (fscanf(load, "%Lf", &mystruct.fifteen_min) != 1)
    {
        fclose(load);
        throw std::runtime_error("could not read load avg 15 min");
    }
    
    if (mystruct.one_min < 0.0L || mystruct.five_min < 0.0L || mystruct.fifteen_min <0.0L) {
        fclose(load);
        throw std::runtime_error("ERROR! HELP! One or more load avreages are negative");
    }
    fclose(load);
    return mystruct;
}