#pragma once

#include "IController.hpp"

namespace route_dsl {

template <typename Controller>
struct RouteBuilder {
  using Handler = http_common::HandlerPtr<Controller>;
  using Desc = typename IController<Controller>::RouteDescription;

  static constexpr Desc GET(const std::string_view path, Handler h) {
    return {http::verb::get, std::string{path}, h};
  }

  static constexpr Desc POST(const std::string_view path, Handler h) {
    return {http::verb::post, std::string{path}, h};
  }

  static constexpr Desc PUT(const std::string_view path, Handler h) {
    return {http::verb::put, std::string{path}, h};
  }

  static constexpr Desc DEL(const std::string_view path, Handler h) {
    return {http::verb::delete_, std::string{path}, h};
  }

  template <typename... R>
  static constexpr auto Routes(R... r) {
    return std::array{r...};
  }

  template <typename... R>
  static constexpr auto WithBase(std::string_view base, R... routes) {
    return std::array{Desc{routes.method, Concat(base, routes.path), routes.handler}...};
  }

 private:
  static constexpr std::string Concat(const std::string_view a,
                                      const std::string_view b) {
    std::string result;
    result.reserve(a.size() + b.size());
    result.append(a).append(b);
    return result;
  }
};

}  // namespace route_dsl
