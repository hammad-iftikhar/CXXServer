#pragma once

#include "Handler.h"
#include "cxxserver.h"

#define BACKLOG 5

class Server
{
private:
    int port;
    int server_fd;
    Handler handler;

public:
    explicit Server();
    ~Server();
    int listen(int port, void (*cb)());
    void get(std::string path, handler_cb cb);
    void post(std::string path, handler_cb cb);
    void put(std::string path, handler_cb cb);
    void del(std::string path, handler_cb cb);
    void patch(std::string path, handler_cb cb);
};
