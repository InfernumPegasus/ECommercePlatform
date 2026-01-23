#pragma once

#include "HttpTypes.hpp"
#include "RequestContext.hpp"

namespace http_common {

template <typename Controller>
using HandlerPtr = Response (Controller::*)(const RequestContext&);

}  // namespace http_common
