#pragma once

#include <iostream>
#include <memory>
#include <stdexcept>

#include "HttpCommon.hpp"
#include "Router.hpp"

template <typename Derived>
class IController : public std::enable_shared_from_this<Derived> {
 public:
  using HandlerType = http_common::HandlerPtr<Derived>;

  struct RouteDescription {
    http::verb method{};
    std::string path{};
    HandlerType handler{};
  };

  void RegisterRoutes(Router& router) {
    const auto instance = this->weak_from_this();
    if (instance.expired()) {
      throw std::logic_error(
          "Controller must be managed by std::shared_ptr before RegisterRoutes()");
    }
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
