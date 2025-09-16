#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include "include/Headers.h"
#include "include/Request.h"
#include "include/Response.h"

Response::Response(int client_fd, Request &request)
    : client_fd(client_fd), status_code(StatusCode::OK), request(request)
{
    headers = Headers();
    set_default_headers();
}

Response::~Response()
{
}

void Response::set_default_headers()
{
    headers.set("Content-Type", "text/plain; charset=utf-8");
    headers.set("Content-Length", "0");
    headers.set("Connection", "close");
    headers.set("Server", "CXXServer");
}

void Response::send(std::string body)
{
    std::string status_code_string = "OK";

    switch (status_code)
    {
    case StatusCode::NOT_FOUND:
        status_code_string = "Not Found";
    case StatusCode::REQUEST_HEADER_FIELDS_TOO_LARGE:
        status_code_string = "Request Header Fields Too Large";
    case StatusCode::NOT_IMPLEMENTED:
        status_code_string = "Not Implemented";
    case StatusCode::BAD_REQUEST:
        status_code_string = "Bad Request";
    case StatusCode::PAYLOAD_TOO_LARGE:
        status_code_string = "Payload Too Large";
    case StatusCode::INTERNAL_SERVER_ERROR:
        status_code_string = "Internal Server Error";
    default:
        break;
    }

    std::string body_string = body + "\n";

    headers.set("Content-Length", std::to_string(body_string.size()));

    std::string response =
        request.http_version + " " + std::to_string(status_code) + " " + status_code_string + "\r\n";

    response += headers.to_string();

    response += "\r\n";
    response += body_string;

    // Ensure entire response is written
    const char *data = response.c_str();
    size_t total = response.size();
    size_t sent = 0;
    while (sent < total)
    {
        ssize_t n = ::send(client_fd, data + sent, total - sent, 0);
        if (n <= 0)
        {
            break;
        }
        sent += static_cast<size_t>(n);
    }

    close(client_fd);
    return;
}

void Response::send()
{
    send("");
}

void Response::status(StatusCode status_code)
{
    this->status_code = status_code;
}