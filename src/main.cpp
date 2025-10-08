#include <filesystem>
#include <iostream>

#include "libcxxserver/include/Request.h"
#include "libcxxserver/include/Response.h"
#include "libcxxserver/include/Server.h"

#define PORT 3000

namespace fs = std::filesystem;

void index_handle(Request req, Response res)
{
    std::cout << "index_handle" << std::endl;

    std::cout << "name=" << req.query["name"][0] << std::endl;
    std::cout << "age=" << req.query["age"][0] << std::endl;
    std::cout << "age=" << req.query["age"][1] << std::endl;

    res.headers.set("Content-Type", "application/json");

    res.send("{\"message\": \"Hello, World!\"}");
}

void upload_handle(Request req, Response res)
{
    std::cout << "upload_handle size=" << req.body_size << std::endl;

    if (!req.files.empty())
    {
        for (auto &file : req.files)
        {
            auto _file = req.files[file.first];

            std::cout << file.first << std::endl;

            for (auto &_file : _file)
            {
                std::cout << "file name=" << _file.filename << " size=" << _file.size << " path=" << _file.temp_path << std::endl;

                fs::copy_file(_file.temp_path, _file.filename);
            }
            std::cout << std::endl;
        }
    }
    else
    {
        std::cout << "no files" << std::endl;
    }

    res.headers.set("Content-Type", "application/json");
    res.send("{\"status\":\"ok\",\"bytes\":" + std::to_string(req.body_size) + "}");
}

void cb()
{
    std::cout << "Server listening on port " << PORT << "\n";
}

void user_handle(Request req, Response res)
{
    std::cout << "test_handle" << std::endl;
    std::cout << "uid=" << req.params["uid"] << std::endl;
    std::cout << "pid=" << req.params["pid"] << std::endl;
    res.headers.set("Content-Type", "application/json");
    res.send("{\"status\":\"ok\"}");
}

int main()
{
    Server server = Server();

    server.get("/", index_handle);
    server.post("/upload", upload_handle);
    server.put("/user/{uid}/posts/{pid}", user_handle);

    server.listen(PORT, cb);
}
