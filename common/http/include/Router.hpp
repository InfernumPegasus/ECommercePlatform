#pragma once

#include <memory>
#include <string_view>

#include "HttpCommon.hpp"
#include "HttpTypes.hpp"
#include "RouteTrie.hpp"

class Router {
 public:
  template <typename Controller>
  void AddRoute(http::verb method, std::string_view path,
                http_common::HandlerPtr<Controller> handler,
                const std::weak_ptr<Controller>& instance) {
    trie_.AddRoute(
        method, path, [handler, instance](const RequestContext& ctx) -> Response {
          const auto locked = instance.lock();
          if (!locked) {
            return MakeErrorResponse(ctx.GetRequest(), http::status::service_unavailable,
                                     "Controller is unavailable");
          }
          return ((*locked).*handler)(ctx);
        });
  }

  template <typename Controller>
  void AddRoute(http::verb method, std::string_view path,
                http_common::HandlerPtr<Controller> handler,
                const std::shared_ptr<Controller>& instance) {
    AddRoute(method, path, handler, std::weak_ptr<Controller>(instance));
  }

  [[nodiscard]] Response Route(const Request& req) const;

  // Для отладки
  void PrintAllRoutes() const;

 private:
  static Response MakeErrorResponse(const Request& req, http::status status,
                                    std::string_view message);

  RouteTrie trie_;
};
