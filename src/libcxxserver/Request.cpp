#include <iostream>
#include <string>
#include <unistd.h>
#include <utility>

#include "include/Headers.h"
#include "include/Request.h"
#include "include/utils.h"

Request::Request(std::string http_message)
{
    this->http_message = std::move(http_message);
    this->parse();
}

Request::~Request()
{
    if (!body_temp_path.empty())
    {
        unlink(body_temp_path.c_str());
    }
}

void Request::parse()
{
    if (this->http_message.empty())
    {
        return;
    }

    std::cout << http_message << std::endl;

    std::vector<std::string> lines = split_string(http_message, "\r\n", true);

    if (lines.empty())
    {
        return;
    }

    std::vector<std::string> first_line_chunks = split_string(lines[0], " ");

    if (first_line_chunks.size() != 3)
    {
        return;
    }

    // Parse first line
    method = to_lowercase(first_line_chunks[0]);
    path = first_line_chunks[1];
    http_version = first_line_chunks[2];

    headers = Headers();

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

            headers.set(header.first, header.second);
        }
    }

    // Parse body
}

Request Request::create_dummy()
{
    return Request("GET / HTTP/1.1\r\n\r\n");
}