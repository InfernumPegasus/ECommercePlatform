#pragma once

#include <array>
#include <concepts>
#include <string>
#include <string_view>
#include <type_traits>

#include "IController.hpp"

namespace route_dsl {

template <typename Controller>
using RouteDesc = typename IController<Controller>::RouteDescription;

template <typename T>
concept RouteLike = requires(T t) {
  t.method;
  t.path;
  t.handler;
};

inline std::string Concat(const std::string_view a, const std::string_view b) {
  std::string result;
  result.reserve(a.size() + b.size());
  result.append(a).append(b);
  return result;
}

template <typename Controller>
constexpr RouteDesc<Controller> GET(const std::string_view path,
                                    http_common::HandlerPtr<Controller> h) {
  return {http::verb::get, std::string{path}, h};
}

template <typename Controller>
constexpr RouteDesc<Controller> POST(const std::string_view path,
                                     http_common::HandlerPtr<Controller> h) {
  return {http::verb::post, std::string{path}, h};
}

template <typename Controller>
constexpr RouteDesc<Controller> PUT(const std::string_view path,
                                    http_common::HandlerPtr<Controller> h) {
  return {http::verb::put, std::string{path}, h};
}

template <typename Controller>
constexpr RouteDesc<Controller> DEL(const std::string_view path,
                                    http_common::HandlerPtr<Controller> h) {
  return {http::verb::delete_, std::string{path}, h};
}

template <RouteLike First, RouteLike... Rest>
constexpr auto Routes(First first, Rest... rest) {
  static_assert((std::same_as<First, Rest> && ...),
                "All routes must have the same descriptor type");
  return std::array{first, rest...};
}

template <RouteLike First, RouteLike... Rest>
constexpr auto WithBase(std::string_view base, First first, Rest... rest) {
  static_assert((std::same_as<First, Rest> && ...),
                "All routes must have the same descriptor type");
  return std::array{First{first.method, Concat(base, first.path), first.handler},
                    First{rest.method, Concat(base, rest.path), rest.handler}...};
}

}  // namespace route_dsl
