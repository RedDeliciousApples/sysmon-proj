#pragma once
struct LoadAverage {
    long double one_min;
    long double five_min;
    long double fifteen_min;
};

LoadAverage get_load_avg();