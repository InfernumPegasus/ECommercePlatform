#pragma once

#include <iostream>

#include "HttpTypes.hpp"
#include "Router.hpp"

struct RouteDesc {
  http::verb method;
  std::string_view path;
};

template <typename Derived>
class IController {
 public:
  void register_routes(Router& router) {
    auto& self = static_cast<Derived&>(*this);

    for (const auto& desc : Derived::routes()) {
      router.add(desc.method, std::string(Derived::base_path()) + std::string(desc.path),
                 [&self, desc](Request const& req) { return self.dispatch(desc, req); });
    }
  }

  void print_available_routes() const {
    const auto& self = static_cast<const Derived&>(*this);

    std::println(std::cout, "Controller routes");
    for (const auto& desc : Derived::routes()) {
      std::println(std::cout, "route: method {}, path {}", http::to_string(desc.method),
                   desc.path);
    }
  }
};
