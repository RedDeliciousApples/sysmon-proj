#include "metrics_sampler.h"
#include "metrics_snapshot.h"
#include "cpu.h"
#include "mem.h"
#include "uptime.h"
#include "loadavg.h"
#include "storage.h"
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <optional>
#include <iostream>
#include <stdexcept>
//constructor
MetricsSampler::MetricsSampler(std::chrono::seconds interval)
    : interval_(interval)
{
    if (interval_ <= std::chrono::seconds::zero()) {
        throw std::invalid_argument("Sampler interval must be positive");
    }
}

//destructor
MetricsSampler::~MetricsSampler(){
    stop();
}

std::optional<MetricsSnapshot> MetricsSampler::latest() const
{
    std::lock_guard<std::mutex> lock(snapshot_mutex_);
    return latest_snapshot_;
}
MetricsSnapshot MetricsSampler::collect_once()
{
    MetricsSnapshot snapshot;
    snapshot.collected_at_unix_seconds = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    const CpuCounters current_cpu = read_cpu_counters();

    if (previous_cpu_) {
        snapshot.cpu_percent =
            calculate_cpu_percent(*previous_cpu_, current_cpu);
    }

    previous_cpu_ = current_cpu;
    snapshot.memory_used_percent = get_mem_usage();
    snapshot.uptime_seconds = time_awake();
    snapshot.load_average = get_load_avg();
    snapshot.filesystems = get_filesystem_usage();
    snapshot.sequence = next_sequence_++;

    return snapshot;
}

void MetricsSampler::start()
{
    if (worker_.joinable()) return;
    
    stopping_ = false;
    worker_ = std::thread(&MetricsSampler::run, this);
}

void MetricsSampler::run()
{
   
    while (!stopping_)
    {
        try{
            MetricsSnapshot snapshot = collect_once();

            {
                std::lock_guard<std::mutex> lock(snapshot_mutex_);
                latest_snapshot_ = snapshot;
            }
        } catch (const std::exception& e) {
            // Handle the exception (e.g., log it)
            std::cerr << "Error collecting metrics: " << e.what() << std::endl;
        }

        std::this_thread::sleep_for(interval_);
    }
}

void MetricsSampler::stop()
{
    stopping_ = true;
    if (worker_.joinable()) {
        worker_.join();
    }
}