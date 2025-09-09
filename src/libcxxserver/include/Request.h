#pragma once

#include "Headers.h"
#include <map>
#include <string>
class Request
{
private:
    std::string http_message;
    void parse();

public:
    explicit Request(std::string http_message);
    ~Request();
    std::string method;
    std::string path;
    std::string body;
    Headers headers;
    std::string query_params;
    std::string cookies;
    std::string protocol;
    std::string host;
    std::string port;
    std::string remote_addr;
    std::string remote_port;
    std::string http_version;
};
