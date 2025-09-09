#include <iostream>

#include "libcxxserver/include/Request.h"
#include "libcxxserver/include/Response.h"
#include "libcxxserver/include/Server.h"

#define PORT 3000

void index_handle(Request req, Response res)
{
    std::cout << "index_handle" << std::endl;

    res.headers.set("Location", "/");

    res.send();
}

void cb()
{
    std::cout << "Server listening on port " << PORT << "\n";
}

int main()
{
    Server server = Server();

    server.get("/", index_handle);

    server.listen(PORT, cb);
}
