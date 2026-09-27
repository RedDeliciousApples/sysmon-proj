#pragma once
#include "../collectors/metrics_sampler.h"

void request_server_shutdown();
void run_server(int port, MetricsSampler& sampler);