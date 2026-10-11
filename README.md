# System Monitor

A lightweight Linux system monitoring project written in C++17, by Christian Saliba and Kai Hoag.

It reads system info from `/proc` and serves it as JSON over a tiny HTTP server. It can also just print things to the terminal if you want that.

## Features

It can read:

- CPU usage from `/proc/stat` (1 second average)
- Memory usage from `/proc/meminfo`
- Load average from `/proc/loadavg`
- Uptime from `/proc/uptime`
- Filesystem usage from `/proc/mounts` plus `statvfs` (currently not output to JSON though)

Plus:

- A background sampler that gets a new snapshot every second
- A small HTTP server with a `/metrics` endpoint
- A browser dashboard
- A Buildroot package so it can be cross-compiled for embedded targets

The server's response will look like this. You can get a response by going to `http://localhost:8080/metrics`.

```json
{
  "cpu_percent": 9.41,
  "memory_used_percent": 8.8,
  "uptime_seconds": 840.93,
  "collected_at_unix_seconds": 1791744005,
  "sequence": 2,
  "load_average": { "one_min": 0.29, "five_min": 0.08, "fifteen_min": 0.02 }
}
```

Notes:

- Anything that isn't `/metrics` will return a 404

## How to build

Linux only, because it uses `/proc`.

```bash
make            # builds build/sysmon
make test       # builds and runs the tests
make run        # builds and runs sysmon
```

Then you can run it like this:

- `./build/sysmon` - print one reading
- `./build/sysmon --watch` - prints every second
- `./build/sysmon --serve` — starts the HTTP server on port 8080

## Dashboard

Use a server (we used python `http.server`) to serve the `frontend` folder, then go to `index.html`

## Embedded (Buildroot)

There's an external Buildroot tree in `buildroot/`. It cross-compiles sysmon so it can run on a small embedded Linux image. In the future this will probably run on QEMU first, then maybe an ARM core.

Eventually we'll add cloud upload, TLS, and running on real hardware. The tests need rewrites too. 

`I Would Rather Be Playing Persona 3 Reload`
