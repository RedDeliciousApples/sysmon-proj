#pragma once

#include "metrics_snapshot.h"
#include "cpu.h"
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <optional>
#include <condition_variable>

class MetricsSampler {
public:
    explicit MetricsSampler(std::chrono::seconds interval);
    ~MetricsSampler();

    void start();
    void stop();

    std::optional<MetricsSnapshot> latest() const;

private:
    void run();
    MetricsSnapshot collect_once();

    std::chrono::seconds interval_;
    std::thread worker_;
    std::atomic<bool> stopping_{false};

    mutable std::mutex snapshot_mutex_;
    std::optional<MetricsSnapshot> latest_snapshot_;

    std::condition_variable stop_cv_;
    std::mutex stop_mutex_;

    std::optional<CpuCounters> previous_cpu_;
    std::uint64_t next_sequence_ = 1;
};

