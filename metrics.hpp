//only include once when compiling
#pragma once

#include "json.hpp"

long double get_cpu_usage();
long double get_mem_usage();
nlohmann::json get_metrics_json();