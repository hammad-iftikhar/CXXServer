#pragma once

#include <netinet/in.h>
#include <string>
#include <vector>

#include "cxxserver.h"

struct Handle
{
    handler_cb cb;
    std::string method;
    std::string path;
};

class Handler
{
private:
    std::vector<struct Handle> handlers;

public:
    explicit Handler();
    ~Handler();
    void register_handler(std::string path, std::string method, handler_cb cb);
    void handle(int client_fd, sockaddr_in client_addr);
};
