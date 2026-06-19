nlohmann::json get_metrics_json() {
    nlohmann::json j;

    j["cpu"] = get_cpu_usage();
    j["mem"] = get_mem_usage();

    return j;
}