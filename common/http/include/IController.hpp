#pragma once

#include <iostream>

#include "HttpTypes.hpp"
#include "Router.hpp"

// Fundamental and minimal route description
struct RouteDesc {
  http::verb method;
  std::string_view path;
};

template <typename Derived>
class IController {
 public:
  void RegisterRoutes(Router& router) {
    auto& self = static_cast<Derived&>(*this);

    for (const auto& desc : Derived::Routes()) {
      router.AddRoute(
          desc.method, std::string(Derived::BasePath()) + desc.path,
          [&self, desc](const Request& req) { return self.Dispatch(desc, req); });
    }
  }

  void PrintAvailableRoutes() const {
    const auto& self = static_cast<const Derived&>(*this);

    std::println(std::cout, "Controller routes:");
    for (const auto& desc : Derived::Routes()) {
      std::println(std::cout, "route: method {}, path {}", http::to_string(desc.method),
                   desc.path);
    }
  }
};
