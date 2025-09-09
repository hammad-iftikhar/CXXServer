#pragma once

#include <string>
#include <netinet/in.h>
#include <vector>

#include "CXXServer.hpp"

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
    void register_handler(std::string, std::string, handler_cb);
    void handle(int, sockaddr_in);
};
