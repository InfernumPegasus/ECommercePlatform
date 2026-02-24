#pragma once

#include <stdexcept>
#include <string>
#include <utility>

#include "HttpTypes.hpp"

struct HttpError {
  http::status status{http::status::internal_server_error};
  std::string code{"internal_error"};
  std::string message{"Internal server error"};
};

class HttpException final : public std::runtime_error {
 public:
  explicit HttpException(HttpError error)
      : std::runtime_error(error.message), error_(std::move(error)) {}

  [[nodiscard]] const HttpError& Error() const noexcept { return error_; }

 private:
  HttpError error_;
};
