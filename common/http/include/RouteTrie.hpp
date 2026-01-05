#pragma once

#include <boost/beast/http.hpp>
#include <functional>
#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

#include "HttpTypes.hpp"

namespace http = boost::beast::http;

class RouteTrie {
 public:
  using Handler = std::function<Response(
      const Request&, const std::unordered_map<std::string, std::string>&)>;

  RouteTrie();
  ~RouteTrie() = default;

  void AddRoute(http::verb method, const std::string& path, Handler handler);

  std::pair<Handler, std::unordered_map<std::string, std::string>> FindRoute(
      http::verb method, const std::string& path) const;

  std::vector<std::string> GetAllRoutes() const;

 private:
  struct TrieNode;

  struct ParamEdge {
    std::string name;
    std::optional<std::regex> pattern;
    std::unique_ptr<TrieNode> child;
  };

  struct TrieNode {
    std::unordered_map<std::string, std::unique_ptr<TrieNode>> static_children;
    std::optional<ParamEdge> param_child;
    std::unordered_map<http::verb, Handler> handlers;
  };

  std::unique_ptr<TrieNode> root_;

  static std::string NormalizePath(std::string path);
  static std::vector<std::string> SplitPath(const std::string& path);

  static bool ParseParamSegment(const std::string& segment, std::string& name,
                                std::optional<std::regex>& pattern);

  void AddPath(const std::vector<std::string>& segments, http::verb method,
               Handler handler);

  std::pair<const TrieNode*, std::unordered_map<std::string, std::string>> FindPath(
      const std::vector<std::string>& segments) const;
};
