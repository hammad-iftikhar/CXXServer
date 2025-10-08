#pragma once

#include "Headers.h"
#include <cstddef>
#include <map>
#include <string>
#include <vector>

struct UploadedFile
{
    std::string field_name;
    std::string filename;
    std::string content_type;
    std::string temp_path;
    std::string temp_name;
    std::size_t size = 0;
};

class Request
{
private:
    std::string http_message;
    void parse();
    void parse_body_content(const std::string &body_content);

public:
    explicit Request(std::string http_message);
    ~Request();
    static Request create_dummy();
    void parse_body();
    std::string method;
    std::string path;
    std::string body;
    std::string body_temp_path;
    std::size_t body_size = 0;
    Headers headers;
    std::map<std::string, std::vector<std::string>> form;
    std::map<std::string, std::vector<UploadedFile>> files;
    std::map<std::string, std::vector<std::string>> query;
    std::string host;
    std::string port;
    std::string remote_addr;
    std::string remote_port;
    std::string http_version;
    std::map<std::string, std::string> params;
};
