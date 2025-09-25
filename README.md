## CXXServer

A minimal C++20 HTTP server with a tiny routing API. The example app listens on port 3000 and registers a simple handler for the `/` path.

### Prerequisites

- **CMake** 3.20+
- **C++20** compiler (e.g., Clang or GCC)
- POSIX sockets support (macOS/Linux). On macOS, install Xcode Command Line Tools.

### Build

Run from the project root:

```bash
./build.sh [release | debug]
```

This generates the build into the `build/` (`build-relase/` in case of release build) directory and produces the `main` executable with libcxxserver library linked.

### Run

From the project root after building:

```bash
./build/main
```

You should see output similar to:

```
Server listening on port 3000
```

### Notes

- Default port: `3000` (see `src/main.cpp`).
- Backlog: `5` (see `include/Server.hpp`).
