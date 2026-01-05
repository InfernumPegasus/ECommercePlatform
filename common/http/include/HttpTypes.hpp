#pragma once

#include <boost/beast/http.hpp>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>

namespace http = boost::beast::http;

using Request = http::request<http::string_body>;
using Response = http::response<http::string_body>;

struct QueryParams {
  std::unordered_map<std::string, std::string> values;

  std::optional<std::string> get(const std::string_view key) const {
    if (const auto it = values.find(std::string(key)); it != values.end()) {
      return it->second;
    }
    return std::nullopt;
  }

  template <typename T>
  std::optional<T> get_as(std::string_view key) const;
};

struct RequestContext {
  const Request& request;
  const std::unordered_map<std::string, std::string>& path_params;
  const QueryParams& query;
};
