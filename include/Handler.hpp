#pragma once

#include <map>
#include <string>
#include <netinet/in.h>
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
    std::map<std::string, struct Handle> handlers;

public:
    explicit Handler();
    ~Handler();
    void register_handler(const char *, const char *, handler_cb);
    void handle(int, sockaddr_in);
};
