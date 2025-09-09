#pragma once

#include <map>
#include <string>
class Request
{
private:
    void parse(const char *);

public:
    explicit Request(const char *);
    ~Request();
    std::string method;
    std::string path;
    std::string body;
    std::map<std::string, std::string> headers;
    std::string query_params;
    std::string cookies;
    std::string protocol;
    std::string host;
    std::string port;
    std::string remote_addr;
    std::string remote_port;
    std::string http_version;
};
