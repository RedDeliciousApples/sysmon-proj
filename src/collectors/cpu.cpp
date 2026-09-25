#include <chrono>
#include <cinttypes>
#include <cstdio>
#include <cmath>
#include <stdexcept>
#include <thread>

#include "cpu.h"

#include "../utils/math_utils.h"


using namespace std::this_thread;
using namespace std::chrono;

std::uint64_t CpuCounters::total() const
{
    return user + nice + system + idle + iowait + irq + softirq + steal;
}

std::uint64_t CpuCounters::idle_total() const
{
    return idle + iowait;
}

CpuCounters read_cpu_counters()
{
    char cpu[4];
    CpuCounters counters{};
    FILE *procstat = fopen("/proc/stat", "r");

    if (procstat == nullptr) {
        throw std::runtime_error("ERROR! HELP! Failed to open /proc/stat");
    }

    const int fields_read = fscanf(
        procstat,
        "%3s %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64 " %" SCNu64
        " %" SCNu64 " %" SCNu64 " %" SCNu64,
        cpu,
        &counters.user,
        &counters.nice,
        &counters.system,
        &counters.idle,
        &counters.iowait,
        &counters.irq,
        &counters.softirq,
        &counters.steal);

    fclose(procstat);

    if (fields_read != 9) {
        throw std::runtime_error("Invalid CPU data in /proc/stat");
    }

    return counters;
}

std::optional<double> calculate_cpu_percent(
    const CpuCounters& previous,
    const CpuCounters& current)
{
    if (current.total() <= previous.total()
        || current.idle_total() < previous.idle_total()) {
        return std::nullopt;
    }

    const std::uint64_t delta_total = current.total() - previous.total();
    const std::uint64_t delta_idle = current.idle_total() - previous.idle_total();

    if (delta_total == 0) {
        return std::nullopt;
    }

    return static_cast<double>(round_to(
        100.0L * (1.0L - static_cast<long double>(delta_idle) / delta_total), 2));
}

long double get_cpu_usage()
{
    const CpuCounters snapshot1 = read_cpu_counters();

    sleep_for(seconds(1));

    const CpuCounters snapshot2 = read_cpu_counters();
    const std::optional<double> cpu_usage = calculate_cpu_percent(snapshot1, snapshot2);

    if (!cpu_usage) {
        throw std::runtime_error("Invalid CPU snapshot: total CPU time did not increase");
    }

    return *cpu_usage;
}
