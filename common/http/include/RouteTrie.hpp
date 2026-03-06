#pragma once

#include <bitset>
#include <boost/container/flat_map.hpp>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <vector>

#include "HttpTypes.hpp"
#include "PathParamType.hpp"
#include "RequestContext.hpp"

class RouteTrie {
 public:
  using Handler = std::function<Response(const RequestContext&)>;
  static constexpr std::size_t kMethodMaskSize = 64;
  using MethodMask = std::bitset<kMethodMaskSize>;

  struct MatchedRoute {
    Handler handler;
    std::unordered_map<std::string, std::string> path_params;
  };

  struct RouteNotFound {};

  struct MethodNotAllowedRoute {
    MethodMask allowed_methods;
  };

  class MatchResult {
   public:
    static MatchResult Matched(Handler handler,
                               std::unordered_map<std::string, std::string> path_params) {
      return MatchResult(MatchedRoute{.handler = std::move(handler),
                                      .path_params = std::move(path_params)});
    }

    static MatchResult NotFound() { return MatchResult(RouteNotFound{}); }

    static MatchResult MethodNotAllowed(MethodMask allowed_methods) {
      return MatchResult(
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

    [[nodiscard]] const MethodMask& AllowedMethods() const {
      return std::get<MethodNotAllowedRoute>(state_).allowed_methods;
    }

   private:
    std::variant<MatchedRoute, RouteNotFound, MethodNotAllowedRoute> state_;

    explicit MatchResult(
        std::variant<MatchedRoute, RouteNotFound, MethodNotAllowedRoute> state)
        : state_(std::move(state)) {}
  };

  RouteTrie();
  ~RouteTrie() = default;

  void AddRoute(http::verb method, std::string_view path, Handler handler);

  [[nodiscard]] MatchResult Match(http::verb method, std::string_view path) const;

  [[nodiscard]] std::vector<std::string> GetAllRoutes() const;

 private:
  struct TrieNode;

  struct TransparentStringHash {
    using is_transparent = void;

    std::size_t operator()(const std::string_view value) const noexcept {
      return std::hash<std::string_view>{}(value);
    }

    std::size_t operator()(const std::string& value) const noexcept {
      return (*this)(std::string_view(value));
    }
  };

  struct TransparentStringEqual {
    using is_transparent = void;

    bool operator()(const std::string_view lhs,
                    const std::string_view rhs) const noexcept {
      return lhs == rhs;
    }

    bool operator()(const std::string& lhs, const std::string& rhs) const noexcept {
      return lhs == rhs;
    }
  };

  struct ParamKey {
    std::string name;
    const PathParamTypeDescriptor* type;

    bool operator<(const ParamKey& other) const {
      return std::tie(type->priority, type->name, name) >
             std::tie(other.type->priority, other.type->name, other.name);
    }

    bool operator==(const ParamKey& other) const = default;
  };

  struct TrieNode {
    std::unordered_map<std::string, std::unique_ptr<TrieNode>, TransparentStringHash,
                       TransparentStringEqual>
        static_children;

    boost::container::flat_map<ParamKey, std::unique_ptr<TrieNode>> param_children;

    std::unordered_map<http::verb, Handler> handlers;
    MethodMask methods_mask;
  };

  std::unique_ptr<TrieNode> root_;

  static std::string NormalizePathOwned(std::string_view path);
  static std::string_view NormalizePathView(std::string_view path);
  static std::vector<std::string> SplitPathOwned(std::string_view path);
  static std::vector<std::string_view> SplitPathView(std::string_view path);

  static bool ParseParamSegment(const std::string& segment, std::string& name,
                                const PathParamTypeDescriptor*& type);

  void AddPath(const std::vector<std::string>& segments, http::verb method,
               Handler handler);

  static void AddMethodToMask(MethodMask& mask, http::verb method);

  [[nodiscard]] std::pair<const TrieNode*, std::unordered_map<std::string, std::string>>
  FindPath(const std::vector<std::string_view>& segments) const;
};
