#include "http_server.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>
#include <stdexcept>

void run_server(int port)
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    std::cout << "Server would start on port " << port << "\n";
}