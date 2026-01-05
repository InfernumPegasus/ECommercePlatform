#pragma once

#include <iostream>

#include "Router.hpp"

template <typename Derived>
class IController {
 public:
  struct RouteWithParams {
    http::verb method;
    std::string_view path;
    Response (Derived::*handler)(
        const Request&, const std::unordered_map<std::string, std::string>&) const;
  };

  struct SimpleRoute {
    http::verb method;
    std::string_view path;
    Response (Derived::*handler)(const Request&) const;
  };

  void RegisterRoutes(Router& router) {
    const auto* instance = static_cast<const Derived*>(this);

    if constexpr (requires { Derived::RoutesWithParams(); }) {
      for (const auto& r : Derived::RoutesWithParams()) {
        const std::string full_path =
            std::string(Derived::BasePath()) + std::string(r.path);
        router.AddRoute(r.method, full_path, r.handler, instance);
      }
    }

    if constexpr (requires { Derived::Routes(); }) {
      for (const auto& r : Derived::Routes()) {
        const std::string full_path =
            std::string(Derived::BasePath()) + std::string(r.path);
        router.AddRoute(r.method, full_path, r.handler, instance);
      }
    }
  }

  void PrintAvailableRoutes() const {
    std::println(std::cout, "Controller for route '{}' routes:", Derived::BasePath());

    if constexpr (requires { Derived::RoutesWithParams(); }) {
      for (const auto& desc : Derived::RoutesWithParams()) {
        std::println(std::cout, "route: method {}, path {}", http::to_string(desc.method),
                     desc.path);
      }
    }

    if constexpr (requires { Derived::Routes(); }) {
      for (const auto& desc : Derived::Routes()) {
        std::println(std::cout, "route: method {}, path {}", http::to_string(desc.method),
                     desc.path);
      }
    }
  }
};
