#pragma once

#include <utility>


long double get_cpu_usage();

struct CpuCounters {
    std::uint64_t user;
    std::uint64_t nice;
    std::uint64_t system;
    std::uint64_t idle;
    std::uint64_t iowait;
    std::uint64_t irq;
    std::uint64_t softirq;
    std::uint64_t steal;

    std::uint64_t total() const;
    std::uint64_t idle_total() const;
};

CpuCounters read_cpu_counters();
std::optional<double> calculate_cpu_percent(
    const CpuCounters& previous,
    const CpuCounters& current);