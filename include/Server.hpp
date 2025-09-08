#pragma once

#include "Handler.hpp"
#include "CXXServer.hpp"

#define BACKLOG 5

class Server
{
private:
    int port;
    int server_fd;
    Handler handler;

public:
    explicit Server(int port);
    ~Server();
    int start();
    void get(const char *, handler_cb);
    void post(const char *, handler_cb);
    void put(const char *, handler_cb);
    void del(const char *, handler_cb);
    void patch(const char *, handler_cb);
};
