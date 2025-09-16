#pragma once

#include "Headers.h"
#include <cstddef>
#include <string>
class Request
{
private:
    std::string http_message;
    void parse();

public:
    explicit Request(std::string http_message);
    ~Request();
    static Request create_dummy();
    std::string method;
    std::string path;
    std::string body;
    std::string body_temp_path;
    std::size_t body_size = 0;
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
