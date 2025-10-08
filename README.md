## CXXServer

A tiny, modern C++20 HTTP server with an intuitive routing API. It uses POSIX sockets and a thread-per-connection model, supports query parsing, path parameters, urlencoded forms, and multipart file uploads, and provides a simple `Request`/`Response` interface.

### What it provides

- **Simple routing**: `get`, `post`, `put`, `patch`, `del` with path params like `/users/{id}`.
- **Request parsing**:
  - Query string into `Request::query` with URL-decoding and support for repeated keys (`std::map<std::string, std::vector<std::string>>`).
  - Form bodies:
    - `application/x-www-form-urlencoded` → `Request::form`.
    - `multipart/form-data` → `Request::form` (text fields) and `Request::files` (uploaded files).
  - Large bodies optionally **spilled to disk** and parsed from a temp file when above a threshold.
- **File uploads**: Each file is exposed as `UploadedFile { field_name, filename, content_type, temp_path, temp_name, size }`.
- **Response helpers**: Set status, headers, and send a body with correct `Content-Length`.
- **Reasonable limits** with clear errors: 431 for oversized headers, 413 for oversized bodies, 501 for chunked transfer (not supported).

### Quick start

#### Prerequisites

- **CMake** 3.20+
- **C++20** compiler (Clang/GCC)
- macOS or Linux (POSIX sockets). On macOS, install Xcode Command Line Tools.

#### Build

From the project root:

```bash
./build.sh [release | debug]
```

Artifacts:

- Debug: `build/main`
- Release: `build-release/main`

#### Run

```bash
./build/main            # or ./build-release/main
```

Expected output:

```
Server listening on port 3000
```

### Usage example

Minimal server from `src/main.cpp`:

```cpp
#include "libcxxserver/include/Request.h"
#include "libcxxserver/include/Response.h"
#include "libcxxserver/include/Server.h"

void index_handle(Request req, Response res) {
  res.headers.set("Content-Type", "application/json");
  res.send("{\"message\": \"Hello, World!\"}");
}

void upload_handle(Request req, Response res) {
  // Access form fields: req.form["field"][0]
  // Access files: req.files["file"][0].temp_path, .filename, .size
  res.headers.set("Content-Type", "application/json");
  res.send("{\"status\":\"ok\"}");
}

void test_handle(Request req, Response res) {
  // Path params
  // req.params["uid"], req.params["name"]
  res.headers.set("Content-Type", "application/json");
  res.send("{\"status\":\"ok\"}");
}

int main() {
  Server server;
  server.get("/", index_handle);
  server.post("/upload", upload_handle);
  server.put("/test/{uid}/game/{name}", test_handle);
  server.listen(3000, [] { std::cout << "Server listening on port 3000\n"; });
}
```

### Try it with curl

- GET with query params (URL-decoded and accessible via `req.query`):

```bash
curl 'http://localhost:3000/?name=alice&age=10&age=20'
```

- PUT with path params (accessible via `req.params`):

```bash
curl -X PUT 'http://localhost:3000/test/123/game/pacman'
```

- POST urlencoded form (parsed into `req.form`):

```bash
curl -X POST 'http://localhost:3000/upload' \
  -H 'Content-Type: application/x-www-form-urlencoded' \
  --data 'title=foo&title=bar&note=hello%20world'
```

- POST multipart upload (files in `req.files`, fields in `req.form`):

```bash
curl -X POST 'http://localhost:3000/upload' \
  -F 'file=@/path/to/local/file1.bin' \
  -F 'file=@/path/to/local/file2.jpg' \
  -F 'title=summer'
```

### API overview

- **Handler signature**: `using handler_cb = void (*)(Request, Response);`

- **Server**

  - `Server::listen(int port, void (*cb)())`
  - `Server::get|post|put|del|patch(std::string path, handler_cb cb)`

- **Request**

  - `std::string method, path, http_version`
  - `Headers headers`
  - `std::map<std::string, std::vector<std::string>> query` (URL-decoded)
  - `std::map<std::string, std::vector<std::string>> form` (for urlencoded/multipart fields)
  - `std::map<std::string, std::vector<UploadedFile>> files`
  - `std::map<std::string, std::string> params` (from route patterns `{name}`)
  - `std::string body`, `std::string body_temp_path`, `std::size_t body_size`

- **UploadedFile**

  - `field_name, filename, content_type, temp_path, temp_name, size`

- **Response**

  - `Headers headers`
  - `void status(StatusCode)`
  - `void send()` / `void send(std::string body)`

- **Headers**
  - `void set(std::string key, std::string value)`
  - `std::string get(std::string key)`

### Configuration and limits

Defaults are in `src/libcxxserver/include/cxxserver.h`:

- `RECV_CHUNK_SIZE = 16 KiB`
- `MAX_HEADER_BYTES = 32 KiB`
- `MAX_BODY_BYTES = 256 MiB`
- `BODY_TO_FILE_THRESHOLD = 8 MiB` (bodies above this spill to a temp file)
- `TEMP_FILE = "/tmp/cxxserver_tmp_XXXXXXXX"`

Other behavior:

- `Connection: close` by default; one request per connection.
- `Transfer-Encoding: chunked` is not supported → 501 Not Implemented.
- When no route matches → 404.
- Backlog: `5` (see `src/libcxxserver/include/Server.h`).
- Concurrency: thread-per-connection.

### Notes & limitations

- No TLS/HTTPS, HTTP/2, or keep-alive pooling.
- No middleware or streaming APIs.
- Works on macOS/Linux (POSIX). Windows is not supported out-of-the-box.

### Development

- The library is built as a shared library `libcxxserver` and linked into `main`.
- `compile_commands.json` is generated for tooling.

### License

This project is provided as-is; add a license if you intend to publish/distribute.
