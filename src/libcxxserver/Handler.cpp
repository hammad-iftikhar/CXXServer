#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "include/Handler.h"
#include "include/Request.h"
#include "include/Response.h"
#include "include/cxxserver.h"

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

    char buffer[MAX_REQUEST_BYTES];

    int bytes = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (bytes <= 0)
    {
        close(client_fd);
        return;
    }
    buffer[bytes] = '\0';

    Request request = Request(std::string(buffer));

    Response response = Response(client_fd, request);

    bool route_found = false;

    for (int i = 0; i < handlers.size(); i++)
    {
        Handle hdl = handlers[i];

        if (hdl.path == request.path && hdl.method == request.method)
        {
            hdl.cb(request, response);
            route_found = true;
            break;
        }
    }

    if (!route_found)
    {
        response.status(StatusCode::NOT_FOUND);
        response.send();
    }

    close(client_fd);
    std::cout << "Connection closed. \n";
}