#include "Server.hpp"

const char *index_handle()
{
    return "hh";
}

int main()
{
    Server server = Server(3000);

    server.get("/", index_handle);

    server.start();
}
