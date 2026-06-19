#include "math_utils.h"

#include <cmath>

long double round_to(long double value, int decimals)
{
    long double factor = std::pow(10.0L, decimals);
    return std::round(value * factor) / factor;
}