#include <iostream>
#include <string>
#include <utility>

#include "Request.hpp"
#include "utils.hpp"

Request::Request(const char *request)
{
    this->parse(request);
}

Request::~Request()
{
}

void Request::parse(const char *content)
{

    std::cout << content << std::endl;

    std::vector<std::string> lines = split_string(content, "\r\n", true);

    std::vector<std::string> first_line_chunks = split_string(lines[0], " ");

    if (first_line_chunks.size() != 3)
    {
        return;
    }

    // Parse first line
    this->method = first_line_chunks[0];
    this->path = first_line_chunks[1];
    this->http_version = first_line_chunks[2];

    // Parse headers
    for (int i = 0; i < lines.size(); i++)
    {

        if (i > 0)
        {
            std::string line = lines[i];

            if (line == "")
            {
                break;
            }

            std::pair<std::string, std::string> header = split_once(line, ": ");

            headers[header.first] = header.second;
        }
    }

    // Parse body
}