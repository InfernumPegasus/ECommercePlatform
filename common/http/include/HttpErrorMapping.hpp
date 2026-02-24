#pragma once

#include <stdexcept>

#include "HttpError.hpp"
#include "TypedParams.hpp"

inline HttpError MapExceptionToHttpError(const std::exception& ex) {
  if (const auto* http_ex = dynamic_cast<const HttpException*>(&ex)) {
    return http_ex->Error();
  }

  if (dynamic_cast<const InvalidParameterException*>(&ex)) {
    return HttpError{.status = http::status::bad_request,
                     .code = "invalid_parameter",
                     .message = ex.what()};
  }

  if (dynamic_cast<const std::invalid_argument*>(&ex)) {
    return HttpError{.status = http::status::bad_request,
                     .code = "invalid_argument",
                     .message = ex.what()};
  }

  if (dynamic_cast<const std::out_of_range*>(&ex)) {
    return HttpError{.status = http::status::bad_request,
                     .code = "out_of_range",
                     .message = ex.what()};
  }

  return HttpError{.status = http::status::internal_server_error,
                   .code = "internal_error",
                   .message = "Internal server error"};
}
