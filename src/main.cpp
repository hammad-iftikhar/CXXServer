#include <iostream>

#include "libcxxserver/include/Request.h"
#include "libcxxserver/include/Response.h"
#include "libcxxserver/include/Server.h"

#define PORT 3000

void index_handle(Request req, Response res)
{
    std::cout << "index_handle" << std::endl;

    res.headers.set("Location", "/");

    res.headers.set("Content-Type", "application/json");

    res.send("{\"message\": \"Hello, World!\"}");
}

void upload_handle(Request req, Response res)
{
    std::cout << "upload_handle size=" << req.body_size;
    if (!req.body_temp_path.empty())
    {
        std::cout << " temp_path=" << req.body_temp_path;
    }
    std::cout << std::endl;

    res.headers.set("Content-Type", "application/json");
    res.send("{\"status\":\"ok\",\"bytes\":" + std::to_string(req.body_size) + "}");
}

void cb()
{
    std::cout << "Server listening on port " << PORT << "\n";
}

int main()
{
    Server server = Server();

    server.get("/", index_handle);
    server.post("/upload", upload_handle);

    server.listen(PORT, cb);
}
