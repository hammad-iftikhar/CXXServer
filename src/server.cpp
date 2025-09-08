#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>

#include "Server.hpp"

Server::Server(int port)
{
    this->port = port;
    this->handler = Handler();
}

Server::~Server()
{
    close(server_fd);
}

int Server::start()
{
    struct sockaddr_in server_addr, client_addr;

    socklen_t client_addr_len = sizeof(client_addr);

    // Initialize  socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    if (server_fd == -1)
    {
        perror("socket initialization failed");
        return 1;
    }

    // Bind socket to an IP/port
    memset(&server_addr, 0, sizeof(server_addr));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(this->port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("socket bind failed");
        close(server_fd);
        return 1;
    }

    // Listen for incoming connections
    if (listen(server_fd, BACKLOG) < 0)
    {
        perror("listen failed");
        close(server_fd);
        return 1;
    }

    std::cout << "Server listening on port " << this->port << "\n";

    // Accept client connections
    while (true)
    {
        int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_addr_len);

        if (client_fd < 0)
        {
            perror("client accept failed");
            continue;
        }

        std::thread(&Handler::handle, &this->handler, client_fd, client_addr).detach();
    }
}

void Server::get(const char *path, handler_cb cb)
{
    this->handler.register_handler(path, "get", cb);
}

void Server::post(const char *path, handler_cb cb)
{
    this->handler.register_handler(path, "post", cb);
}

void Server::put(const char *path, handler_cb cb)
{
    this->handler.register_handler(path, "put", cb);
}

void Server::del(const char *path, handler_cb cb)
{
    this->handler.register_handler(path, "delete", cb);
}

void Server::patch(const char *path, handler_cb cb)
{
    this->handler.register_handler(path, "patch", cb);
}