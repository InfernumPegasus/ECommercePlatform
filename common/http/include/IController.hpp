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
    std::string path{};
    HandlerType handler{};
  };

  void RegisterRoutes(Router& router) {
    auto* instance = static_cast<Derived*>(this);
    for (const auto& r : Derived::Routes()) {
      router.AddRoute(r.method, r.path, r.handler, instance);
    }
  }

  void PrintAvailableRoutes() const {
    for (const auto& desc : Derived::Routes()) {
      std::println(std::cout, "route: method {}, path {}", http::to_string(desc.method),
                   desc.path);
    }
  }
};
