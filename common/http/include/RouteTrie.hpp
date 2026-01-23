#pragma once

#include <boost/beast/http.hpp>
#include <boost/container/flat_map.hpp>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "HttpTypes.hpp"
#include "PathParamType.hpp"
#include "RequestContext.hpp"

namespace http = boost::beast::http;

class RouteTrie {
 public:
  using Handler = std::function<Response(const RequestContext&)>;

  RouteTrie();
  ~RouteTrie() = default;

  void AddRoute(http::verb method, std::string_view path, Handler handler);

  [[nodiscard]] std::pair<Handler, std::unordered_map<std::string, std::string>>
  FindRoute(http::verb method, std::string_view path) const;

  [[nodiscard]] std::vector<std::string> GetAllRoutes() const;

 private:
  struct TrieNode;

  struct ParamKey {
    std::string name;
    const PathParamTypeDescriptor* type;

    bool operator<(const ParamKey& other) const {
      return type->priority > other.type->priority;
    }

    bool operator==(const ParamKey& other) const = default;
  };

  struct TrieNode {
    std::unordered_map<std::string, std::unique_ptr<TrieNode>> static_children;

    boost::container::flat_map<ParamKey, std::unique_ptr<TrieNode>> param_children;

    std::unordered_map<http::verb, Handler> handlers;
  };

  std::unique_ptr<TrieNode> root_;

  static std::string NormalizePath(std::string_view path);
  static std::vector<std::string> SplitPath(std::string_view path);

  static bool ParseParamSegment(const std::string& segment, std::string& name,
                                const PathParamTypeDescriptor*& type);

  void AddPath(const std::vector<std::string>& segments, http::verb method,
               Handler handler);

  [[nodiscard]] std::pair<const TrieNode*, std::unordered_map<std::string, std::string>>
  FindPath(const std::vector<std::string>& segments) const;
};
