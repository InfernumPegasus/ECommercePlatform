#pragma once

#include "HttpCommon.hpp"
#include "HttpTypes.hpp"
#include "RouteTrie.hpp"

class Router {
 public:
  template <typename Controller>
  void AddRoute(http::verb method, std::string_view path,
                http_common::HandlerPtr<Controller> handler, Controller* instance) {
    trie_.AddRoute(method, path,
                   [handler, instance](const RequestContext& ctx) -> Response {
                     return (instance->*handler)(ctx);
                   });
  }

  [[nodiscard]] Response Route(const Request& req) const;

  // Для отладки
  void PrintAllRoutes() const;

 private:
  RouteTrie trie_;
};
