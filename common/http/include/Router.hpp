#pragma once

#include <string>
#include <unordered_map>

#include "HttpTypes.hpp"

class Router {
 public:
  void AddRoute(http::verb method, std::string_view path, Handler handler);

  Response Route(const Request& req) const;

 private:
  using Key = std::pair<http::verb, std::string>;

  struct KeyHash {
    std::size_t operator()(const Key& k) const noexcept {
      return std::hash<int>()(static_cast<int>(k.first)) ^
             std::hash<std::string>()(k.second);
    }
  };

  std::unordered_map<Key, Handler, KeyHash> routes_;
};
