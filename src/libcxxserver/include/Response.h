#pragma once

#include <map>
#include <string>

#include "Headers.h"
#include "Request.h"

class Response
{
private:
    int client_fd;
    int status_code;
    Request &request;
    void set_default_headers();

public:
    explicit Response(int client_fd, Request &request);
    ~Response();
    Headers headers;
    void send();
    void status(int status_code);
};
