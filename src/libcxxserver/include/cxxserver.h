#pragma once

#include "Request.h"
#include "Response.h"
#include <cstddef>

// Keep request buffer reasonable for stack allocation
constexpr std::size_t MAX_REQUEST_BYTES = 64 * 1024;

using handler_cb = void (*)(Request, Response);