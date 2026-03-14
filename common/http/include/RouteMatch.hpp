#pragma once

#include <array>
#include <bitset>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>

#include "HttpTypes.hpp"
#include "RequestContext.hpp"

using RouteHandler = std::move_only_function<Response(const RequestContext&) const>;
using PathParams = std::unordered_map<std::string, std::string>;

inline constexpr auto kTrackedHttpMethods =
    std::to_array({http::verb::get, http::verb::head, http::verb::post, http::verb::put,
                   http::verb::delete_, http::verb::connect, http::verb::options,
                   http::verb::trace, http::verb::patch});

using RouteMethodMask = std::bitset<kTrackedHttpMethods.size()>;

inline std::optional<std::size_t> MethodToMaskIndex(const http::verb method) {
  for (std::size_t i = 0; i < kTrackedHttpMethods.size(); ++i) {
    if (kTrackedHttpMethods[i] == method) {
      return i;
    }
  }
  return std::nullopt;
}

inline void MarkMethodInMask(RouteMethodMask& mask, const http::verb method) {
  if (const auto idx = MethodToMaskIndex(method); idx.has_value()) {
    mask.set(*idx);
  }
}

inline bool MaskContainsMethod(const RouteMethodMask& mask, const http::verb method) {
  if (const auto idx = MethodToMaskIndex(method); idx.has_value()) {
    return mask.test(*idx);
  }
  return false;
}

inline std::string MethodMaskToAllowHeader(const RouteMethodMask& mask) {
  std::string allow_header;
  bool first = true;

  for (std::size_t i = 0; i < kTrackedHttpMethods.size(); ++i) {
    if (!mask.test(i)) {
      continue;
    }

    if (!first) {
      allow_header += ", ";
    }

    allow_header += http::to_string(kTrackedHttpMethods[i]);
    first = false;
  }

  return allow_header;
}

struct MatchedRoute {
  const RouteHandler* handler;
  PathParams path_params;

  [[nodiscard]] Response Invoke(const RequestContext& ctx) const {
    return (*handler)(ctx);
  }
};

struct RouteNotFound {};

struct MethodNotAllowedRoute {
  RouteMethodMask allowed_methods;
};

class RouteMatchResult {
 public:
  static RouteMatchResult Matched(const RouteHandler* handler, PathParams path_params) {
    return RouteMatchResult(
        MatchedRoute{.handler = handler, .path_params = std::move(path_params)});
  }

  static RouteMatchResult NotFound() { return RouteMatchResult(RouteNotFound{}); }

  static RouteMatchResult MethodNotAllowed(RouteMethodMask allowed_methods) {
    return RouteMatchResult(
        MethodNotAllowedRoute{.allowed_methods = std::move(allowed_methods)});
  }

  [[nodiscard]] bool IsMatched() const {
    return std::holds_alternative<MatchedRoute>(state_);
  }

  [[nodiscard]] bool IsNotFound() const {
    return std::holds_alternative<RouteNotFound>(state_);
  }

  [[nodiscard]] bool IsMethodNotAllowed() const {
    return std::holds_alternative<MethodNotAllowedRoute>(state_);
  }

  [[nodiscard]] const MatchedRoute& AsMatched() const {
    return std::get<MatchedRoute>(state_);
  }

  [[nodiscard]] MatchedRoute& AsMatched() { return std::get<MatchedRoute>(state_); }

  [[nodiscard]] const RouteMethodMask& AllowedMethods() const {
    return std::get<MethodNotAllowedRoute>(state_).allowed_methods;
  }

  [[nodiscard]] bool AllowsMethod(const http::verb method) const {
    if (!IsMethodNotAllowed()) {
      return false;
    }
    return MaskContainsMethod(AllowedMethods(), method);
  }

 private:
  std::variant<MatchedRoute, RouteNotFound, MethodNotAllowedRoute> state_;

  explicit RouteMatchResult(
      std::variant<MatchedRoute, RouteNotFound, MethodNotAllowedRoute> state)
      : state_(std::move(state)) {}
};
