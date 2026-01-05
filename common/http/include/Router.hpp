#pragma once

#include <boost/beast/http.hpp>
#include <string>
#include <unordered_map>

#include "HttpTypes.hpp"
#include "RouteTrie.hpp"

class Router {
 public:
  // Хендлер с параметрами
  template <typename Controller>
  void AddRoute(http::verb method, const std::string& path,
                Response (Controller::*handler)(
                    const Request&, const std::unordered_map<std::string, std::string>&)
                    const,
                const Controller* instance) {
    trie_.AddRoute(
        method, path,
        [handler, instance](const Request& req,
                            const std::unordered_map<std::string, std::string>& params)
            -> Response { return (instance->*handler)(req, params); });
  }

  // Хендлер без параметров
  template <typename Controller>
  void AddRoute(http::verb method, const std::string& path,
                Response (Controller::*handler)(const Request&) const,
                const Controller* instance) {
    trie_.AddRoute(method, path,
                   [handler, instance](
                       const Request& req,
                       const std::unordered_map<std::string, std::string>&) -> Response {
                     return (instance->*handler)(req);
                   });
  }

  Response Route(const Request& req) const;

  // Для отладки
  void PrintAllRoutes() const;

 private:
  RouteTrie trie_;
};
