#include "../src/collectors/metrics_sampler.h"

#include <cassert>
#include <chrono>
#include <iostream>
#include <thread>

int sampler_test()
{
    using namespace std::chrono_literals;

    MetricsSampler sampler(1s);
    sampler.start();

    std::this_thread::sleep_for(250ms);

    const auto first = sampler.latest();
    assert(first.has_value());
    assert(!first->cpu_percent.has_value());

    std::cout << "First snapshot: sequence=" << first->sequence
              << ", CPU is warming up\n";

    std::this_thread::sleep_for(1100ms);

    const auto second = sampler.latest();
    assert(second.has_value());
    assert(second->sequence > first->sequence);
    assert(second->cpu_percent.has_value());

    std::cout << "Second snapshot: sequence=" << second->sequence
              << ", CPU=" << *second->cpu_percent << "%\n";

    sampler.stop();
}