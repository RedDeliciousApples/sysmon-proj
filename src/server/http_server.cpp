#include "http_server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <cstring>
#include <csignal>

#include "../utils/getjson.h"
#include "../collectors/metrics_sampler.h"

namespace {
volatile std::sig_atomic_t shutdown_requested = 0;

bool send_all(int client_fd, const std::string& response)
{
    std::size_t total_sent = 0;
    while (total_sent < response.size()) {
        const ssize_t sent = send(
            client_fd,
            response.data() + total_sent,
            response.size() - total_sent,
            MSG_NOSIGNAL);

        if (sent < 0) {
            if (errno == EINTR) {
                continue;
            }
            return false;
        }

        if (sent == 0) {
            errno = EPIPE;
            return false;
        }

        total_sent += static_cast<std::size_t>(sent);
    }

    return true;
}

void log_send_error()
{
    std::cerr << "Failed to send response: " << std::strerror(errno) << '\n';
}
}

void request_server_shutdown()
{
    shutdown_requested = 1;
}

void run_server(int port, MetricsSampler& sampler)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd < 0) {
        throw std::runtime_error("Failed to create socket");
    }

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(server_fd);
        throw std::runtime_error("Failed to set socket options");
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port); //convert to network byte order

    if (bind(server_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        close(server_fd);
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(server_fd, 10) < 0) {
        close(server_fd);
        throw std::runtime_error("Failed to listen on socket");
    }

    std::cout << "sysmon server listening on http://localhost:" << port << "\n";

    while (!shutdown_requested) {
        //REVIEW. appears to work but needs more testing, and multiconnection support
        pollfd server_poll{};
        server_poll.fd = server_fd;
        server_poll.events = POLLIN;
        const int poll_result = poll(&server_poll, 1, 250);

        if (poll_result < 0) {
            if (errno == EINTR) {
                continue;
            }
            close(server_fd);
            throw std::runtime_error("Failed to poll listening socket");
        }

        if (poll_result == 0 || !(server_poll.revents & POLLIN)) {
            continue;
        }

        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, reinterpret_cast<sockaddr*>(&client_addr), &client_len);

        if (client_fd < 0) {
            // accept failed; continue accepting next connection
            continue;
        }

        // read request (simple, single recv is sufficient for our small requests)
        char buffer[4096];
        ssize_t received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
        if (received <= 0) {
            close(client_fd);
            continue;
        }
        buffer[received] = '\0';

        // parse request-line: METHOD PATH VERSION
        std::istringstream reqstream(buffer);
        std::string method;
        std::string path;
        std::string version;
        reqstream >> method >> path >> version;

        if (method == "GET" && path == "/metrics") {
            try {
                nlohmann::json j;
                const auto snapshot = sampler.latest();

                if (!snapshot) {
                    const std::string body = "{\"error\":\"Metrics unavailable\"}";
                    std::ostringstream resp;
                    resp << "HTTP/1.1 503 Service Unavailable\r\n";
                    resp << "Content-Type: application/json\r\n";
                    resp << "Content-Length: " << body.size() << "\r\n";
                    resp << "Connection: close\r\n";
                    resp << "\r\n";
                    resp << body;
                    const std::string out = resp.str();
                    if (!send_all(client_fd, out)) {
                        log_send_error();
                    }
                } else {
                    j = get_metrics_json_mine(*snapshot);
                    std::string body = j.dump();

                    std::ostringstream resp;
                    resp << "HTTP/1.1 200 OK\r\n";
                    resp << "Content-Type: application/json\r\n";
                    resp << "Content-Length: " << body.size() << "\r\n";
                    resp << "Connection: close\r\n";
                    resp << "\r\n";
                    resp << body;

                    std::string out = resp.str();
                    if (!send_all(client_fd, out)) {
                        log_send_error();
                    }
                }
            } catch (const std::exception &e) {
                const std::string body = std::string("{\"error\":\"") + e.what() + "\"}";
                std::ostringstream resp;
                resp << "HTTP/1.1 500 Internal Server Error\r\n";
                resp << "Content-Type: application/json\r\n";
                resp << "Content-Length: " << body.size() << "\r\n";
                resp << "Connection: close\r\n";
                resp << "\r\n";
                resp << body;
                std::string out = resp.str();
                if (!send_all(client_fd, out)) {
                    log_send_error();
                }
            }
        } else {
            const std::string body = "Not Found";
            std::ostringstream resp;
            resp << "HTTP/1.1 404 Not Found\r\n";
            resp << "Content-Type: text/plain\r\n";
            resp << "Content-Length: " << body.size() << "\r\n";
            resp << "Connection: close\r\n";
            resp << "\r\n";
            resp << body;
            std::string out = resp.str();
            if (!send_all(client_fd, out)) {
                log_send_error();
            }
        }

        close(client_fd);
    }

    std::cout << "Shutting down sysmon server\n";
    close(server_fd);
}