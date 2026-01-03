#pragma once

#include <boost/beast/http.hpp>
#include <functional>
#include <string>
#include <unordered_map>

#include "HttpTypes.hpp"

struct RouteDesc {
  http::verb method;
  std::string_view path;
};

class Router {
 public:
  template <typename Controller>
  void AddRoute(http::verb method, const std::string& path,
                Response (Controller::*handler)(const Request&) const,
                const Controller* instance) {
    routes_.emplace(std::make_pair(method, NormalizePath(path)),
                    [handler, instance](const Request& req) -> Response {
                      return (instance->*handler)(req);
                    });
  }

  Response Route(const Request& req) const;

 private:
  using Key = std::pair<http::verb, std::string>;
  using Handler = std::function<Response(const Request&)>;

  struct KeyHash {
    std::size_t operator()(const Key& k) const noexcept {
      return std::hash<int>{}(static_cast<int>(k.first)) ^
             std::hash<std::string>{}(k.second);
    }
  };

  // TODO use trie instead
  std::unordered_map<Key, Handler, KeyHash> routes_;

  static std::string NormalizePath(std::string_view path) {
    if (path.size() > 1 && path.ends_with('/')) {
      path.remove_suffix(1);
    }
    return std::string(path);
  }
};
