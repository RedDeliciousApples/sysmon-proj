#include "http_server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <cstring>

#include "../utils/getjson.h"

void run_server(int port)
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

    while (true) {
        //REVIEW. appears to work but needs more testing, and multiconnection support
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
                nlohmann::json j = get_metrics_json();
                std::string body = j.dump();

                std::ostringstream resp;
                resp << "HTTP/1.1 200 OK\r\n";
                resp << "Content-Type: application/json\r\n";
                resp << "Content-Length: " << body.size() << "\r\n";
                resp << "Connection: close\r\n";
                resp << "\r\n";
                resp << body;

                std::string out = resp.str();
                send(client_fd, out.c_str(), out.size(), 0);
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
                send(client_fd, out.c_str(), out.size(), 0);
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
            send(client_fd, out.c_str(), out.size(), 0);
        }

        close(client_fd);
    }

    close(server_fd);
}