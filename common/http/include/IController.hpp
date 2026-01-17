#pragma once

#include <iostream>

#include "HttpCommon.hpp"
#include "Router.hpp"

template <typename Derived>
class IController {
 public:
  using HandlerType = http_common::HandlerPtr<Derived>;

  struct RouteDescription {
    http::verb method{};
    std::string_view path{};
    HandlerType handler{};
  };

  void RegisterRoutes(Router& router) {
    auto* instance = static_cast<Derived*>(this);
    for (const auto& r : Derived::Routes()) {
      const std::string full_path =
          std::string(Derived::BasePath()) + std::string(r.path);
      router.AddRoute(r.method, full_path, r.handler, instance);
    }
  }

  void PrintAvailableRoutes() const {
    std::println(std::cout, "Controller for route '{}' routes:", Derived::BasePath());

    for (const auto& desc : Derived::Routes()) {
      std::println(std::cout, "route: method {}, path {}", http::to_string(desc.method),
                   desc.path);
    }
  }
};
