#pragma once
#include "json.hpp"
#include "../collectors/metrics_snapshot.h"
nlohmann::json get_metrics_json_mine(const MetricsSnapshot& snapshot);