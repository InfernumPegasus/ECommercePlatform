#pragma once

#include <boost/beast/http.hpp>
#include <string>

#include "HttpTypes.hpp"
#include "RouteTrie.hpp"

class Router {
 public:
  template <typename Controller>
  void AddRoute(http::verb method, std::string_view path,
                Response (Controller::*handler)(const RequestContext&) const,
                const Controller* instance) {
    trie_.AddRoute(method, path,
                   [handler, instance](const RequestContext& ctx) -> Response {
                     return (instance->*handler)(ctx);
                   });
  }

  Response Route(const Request& req) const;

  // Для отладки
  void PrintAllRoutes() const;

 private:
  RouteTrie trie_;
};