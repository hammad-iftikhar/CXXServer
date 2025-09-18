#pragma once

#include "Request.h"
#include "Response.h"
#include <cstddef>
#include <string>

// Receive parameters and limits
constexpr std::size_t RECV_CHUNK_SIZE = 16 * 1024;              // 16 KiB per recv()
constexpr std::size_t MAX_HEADER_BYTES = 32 * 1024;             // 32 KiB max header size
constexpr std::size_t MAX_BODY_BYTES = 256 * 1024 * 1024;       // 256 MiB max body size
constexpr std::size_t BODY_TO_FILE_THRESHOLD = 8 * 1024 * 1024; // spill to disk beyond 8 MiB
const std::string TEMP_FILE = "/tmp/cxxserver_tmp_XXXXXXXX";

using handler_cb = void (*)(Request, Response);