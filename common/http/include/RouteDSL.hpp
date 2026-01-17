#pragma once

#include "IController.hpp"

namespace route_dsl {

template <typename Controller>
struct RouteBuilder {
  using Handler = http_common::HandlerPtr<Controller>;
  using Desc = typename IController<Controller>::RouteDescription;

  static constexpr Desc GET(std::string_view path, Handler h) {
    return {http::verb::get, path, h};
  }

  static constexpr Desc POST(std::string_view path, Handler h) {
    return {http::verb::post, path, h};
  }

  static constexpr Desc PUT(std::string_view path, Handler h) {
    return {http::verb::put, path, h};
  }

  static constexpr Desc DEL(std::string_view path, Handler h) {
    return {http::verb::delete_, path, h};
  }

  template <typename... R>
  static constexpr auto Routes(R... r) {
    return std::array{r...};
  }
};

}  // namespace route_dsl
