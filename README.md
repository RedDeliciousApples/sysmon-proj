# System Monitor

A lightweight Linux system monitoring project written in C/C++, by Christian Saliba and Kai Hoag.

## Goal

Read system information from `/proc` and calculate
- CPU usage from `/proc/stat`
- Memory usage from `/proc/meminfo`
- Uptime from `/proc/uptime` (someday)

Eventually (sometime in the far future), the monitoring agent will send data to a cloud server, and a web dashboard will display the results.

## Current Features

- Reads CPU data from `/proc/stat`
- Computes CPU usage over time (1 second right now, can be changed)
- Reads memory data from `/proc/meminfo`

## How to Build
Linux only because it uses /proc.

Linux environment:
```bash
g++ -std=c++17 -Iinclude -Iexternal src/main.cpp src/metrics.cpp -o main
```
```I Would Rather Be Playing Persona 3 Reload```
