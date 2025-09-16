#include <arpa/inet.h>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "include/Handler.h"
#include "include/Request.h"
#include "include/Response.h"
#include "include/cxxserver.h"

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

            response.send("Headers too large");

            close(client_fd);

            std::cout << "Connection closed. \n";
            return;
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
        }
        else
        {
            // Spill to temp file to avoid large heap usage
            char tmpPath[] = "/tmp/cxxserver_body_XXXXXX";
            int tmpFd = mkstemp(tmpPath);
            if (tmpFd == -1)
            {
                response.status(StatusCode::INTERNAL_SERVER_ERROR);
                return response.send("Failed to create temp file");
            }

            request.body_temp_path = std::string(tmpPath);

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
        }
    }

    bool route_found = false;

    for (int i = 0; i < handlers.size(); i++)
    {
        Handle hdl = handlers[i];

        if (hdl.path == request.path && hdl.method == request.method)
        {
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