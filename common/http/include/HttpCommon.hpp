#pragma once

#include "HttpTypes.hpp"
#include "RequestContext.hpp"

namespace http_common {

using HandlerSignature = Response(const RequestContext&);
template <typename Controller>
using HandlerPtr = Response (Controller::*)(const RequestContext&);

}  // namespace http_common
