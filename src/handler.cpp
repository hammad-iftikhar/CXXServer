#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "Handler.hpp"

Handler::Handler() {}

Handler::~Handler() {}

void Handler::register_handler(const char *path, const char *method, handler_cb cb)
{
    std::string key(path);
    this->handlers[key] = Handle{cb, std::string(method), std::string(path)};
}

void Handler::handle(int client_fd, sockaddr_in client_addr)
{

    std::cout << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << "\n";

    const char *msg = "Hello from server\n";
    send(client_fd, msg, strlen(msg), 0);

    char buffer[1024];

    int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes > 0)
    {
        buffer[bytes] = '\0';
    }

    std::cout << "Received buffer: " << buffer << std::endl;

    auto it = handlers.find("/");
    if (it != handlers.end() && it->second.cb)
    {
        const char *resp = it->second.cb();
        if (resp)
        {
            send(client_fd, resp, strlen(resp), 0);
        }
    }

    close(client_fd);
    std::cout << "Connection closed. \n";
}