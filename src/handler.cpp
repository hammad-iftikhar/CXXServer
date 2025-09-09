#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "CXXServer.hpp"
#include "Handler.hpp"
#include "Request.hpp"

Handler::Handler() {}

Handler::~Handler() {}

void Handler::register_handler(std::string path, std::string method, handler_cb cb)
{
    std::string key(path);

    Handle hdl = Handle{cb, std::string(method), std::string(path)};

    this->handlers.push_back(hdl);
}

void Handler::handle(int client_fd, sockaddr_in client_addr)
{

    std::cout << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << "\n";

    const char *msg = "Hello from server\n";
    send(client_fd, msg, strlen(msg), 0);

    char buffer[MAX_REQUEST_BYTES];

    int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes > 0)
    {
        buffer[bytes] = '\0';
    }

    Request request = Request(buffer);

    for (int i = 0; i < handlers.size(); i++)
    {
        Handle hdl = handlers[i];

        if (hdl.path == request.path && hdl.method == request.method)
        {
            const char *resp = hdl.cb();

            if (resp)
            {
                send(client_fd, resp, strlen(resp), 0);
            }

            break;
        }
    }

    close(client_fd);
    std::cout << "Connection closed. \n";
}