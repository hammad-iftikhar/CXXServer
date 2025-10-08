#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <map>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "include/Handler.h"
#include "include/Request.h"
#include "include/Response.h"
#include "include/cxxserver.h"
#include "include/utils.h"

Handler::Handler() {}

Handler::~Handler() {}

void Handler::register_handler(std::string path, std::string method, handler_cb cb)
{
    std::string key(path);

    Handle hdl = Handle{cb, std::string(method), std::string(path)};

    this->handlers.push_back(hdl);
}

void Handler::handle(int client_fd, sockaddr_in client_addr)
{

    std::cout << "New connection from " << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << "\n";

    // Read headers first
    std::string recv_buffer;
    recv_buffer.reserve(8 * 1024);

    bool headers_complete = false;
    size_t header_end_pos = std::string::npos;

    while (!headers_complete)
    {
        char chunk[RECV_CHUNK_SIZE];
        int bytes = recv(client_fd, chunk, sizeof(chunk), 0);
        if (bytes <= 0)
        {
            close(client_fd);
            return;
        }
        recv_buffer.append(chunk, static_cast<size_t>(bytes));

        if (recv_buffer.size() > MAX_HEADER_BYTES)
        {
            // Headers too large

            // Create a dummy request to pass in reponse
            Request dummy = Request::create_dummy();

            Response response(client_fd, dummy);

            response.status(StatusCode::REQUEST_HEADER_FIELDS_TOO_LARGE);

            return response.send("Headers too large");
        }

        header_end_pos = recv_buffer.find("\r\n\r\n");

        if (header_end_pos != std::string::npos)
        {
            headers_complete = true;
        }
    }

    // Separate headers and any already-received body bytes
    const size_t headers_len = header_end_pos + 4; // include CRLFCRLF
    const std::string header_text = recv_buffer.substr(0, headers_len);
    const std::string initial_body = recv_buffer.substr(headers_len);

    // Parse request line and headers
    Request request(header_text);
    Response response(client_fd, request);

    // Determine body handling
    std::string content_length_value;
    std::string transfer_encoding = request.headers.get("Transfer-Encoding");

    if (!transfer_encoding.empty() && transfer_encoding == "chunked")
    {
        response.status(StatusCode::NOT_IMPLEMENTED);
        return response.send("Chunked transfer encoding is not supported");
    }

    content_length_value = request.headers.get("Content-Length");

    size_t content_length = 0;

    if (!content_length_value.empty())
    {
        try
        {
            content_length = static_cast<size_t>(std::stoull(content_length_value));
        }
        catch (...)
        {
            response.status(StatusCode::BAD_REQUEST);
            return response.send("Invalid Content-Length");
        }

        if (content_length > MAX_BODY_BYTES)
        {
            response.status(StatusCode::PAYLOAD_TOO_LARGE);
            return response.send("Payload too large");
        }

        request.body_size = content_length;

        // Decide storage strategy
        if (content_length <= BODY_TO_FILE_THRESHOLD)
        {
            request.body.clear();
            request.body.reserve(content_length);
            request.body.append(initial_body);

            while (request.body.size() < content_length)
            {
                char chunk[RECV_CHUNK_SIZE];
                int bytes = recv(client_fd, chunk, sizeof(chunk), 0);
                if (bytes <= 0)
                {
                    break;
                }
                request.body.append(chunk, static_cast<size_t>(bytes));

                if (request.body.size() > MAX_BODY_BYTES)
                {
                    response.status(StatusCode::PAYLOAD_TOO_LARGE);
                    return response.send("Payload too large");
                }
            }
            // Parse small body immediately
            request.parse_body();
        }
        else
        {
            // Spill to temp file to avoid large heap usage
            std::string tmp_template = TEMP_FILE;
            std::vector<char> tmpl(tmp_template.begin(), tmp_template.end());
            tmpl.push_back('\0');
            int tmpFd = mkstemp(tmpl.data());
            if (tmpFd == -1)
            {
                response.status(StatusCode::INTERNAL_SERVER_ERROR);
                return response.send("Failed to create temp file");
            }

            request.body_temp_path = std::string(tmpl.data());

            // Write any already read bytes first
            if (!initial_body.empty())
            {
                ssize_t w = write(tmpFd, initial_body.data(), initial_body.size());
                (void)w; // ignore short writes handling for simplicity
            }

            size_t written = initial_body.size();
            while (written < content_length)
            {
                char chunk[RECV_CHUNK_SIZE];
                int bytes = recv(client_fd, chunk, sizeof(chunk), 0);
                if (bytes <= 0)
                {
                    break;
                }
                ssize_t w = write(tmpFd, chunk, static_cast<size_t>(bytes));
                (void)w;
                written += static_cast<size_t>(bytes);

                if (written > MAX_BODY_BYTES)
                {
                    close(tmpFd);
                    response.status(StatusCode::PAYLOAD_TOO_LARGE);
                    return response.send("Payload too large");
                }
            }
            close(tmpFd);

            // Parse large body from temp file
            request.parse_body();
        }
    }

    bool route_found = false;

    // Helper: match route pattern like "/endpoint/{param}" and extract params
    auto match_route_and_extract = [](const std::string &pattern, const std::string &path,
                                      std::map<std::string, std::string> &out_params) -> bool
    {
        std::vector<std::string> pattern_segments = split_string(pattern, "/");
        std::vector<std::string> path_segments = split_string(path, "/");

        if (pattern_segments.size() != path_segments.size())
        {
            return false;
        }

        for (size_t i = 0; i < pattern_segments.size(); ++i)
        {
            const std::string &p = pattern_segments[i];
            const std::string &s = path_segments[i];

            if (!p.empty() && p.front() == '{' && p.back() == '}' && p.size() > 2)
            {
                std::string name = p.substr(1, p.size() - 2);
                // store decoded value to be consistent with query decoding
                std::string value = url_decode(s);
                out_params[name] = value;
                continue;
            }

            if (p != s)
            {
                return false;
            }
        }

        return true;
    };

    for (int i = 0; i < handlers.size(); i++)
    {
        Handle hdl = handlers[i];

        std::map<std::string, std::string> matched_params;
        if (hdl.method == request.method && match_route_and_extract(hdl.path, request.path, matched_params))
        {
            request.params = std::move(matched_params);
            hdl.cb(request, response);
            route_found = true;
            break;
        }
    }

    if (!route_found)
    {
        response.status(StatusCode::NOT_FOUND);
        response.send();
    }

    close(client_fd);
    std::cout << "Connection closed. \n";
}