#include "RouteTrie.hpp"

#include <iostream>
#include <ranges>

#include "PathParamTypeRegistry.hpp"

RouteTrie::RouteTrie() : root_(std::make_unique<TrieNode>()) {}

std::string RouteTrie::NormalizePath(std::string_view path) {
  if (!path.empty() && path.front() == '/') {
    path.remove_prefix(1);
  }
  if (!path.empty() && path.back() == '/') {
    path.remove_suffix(1);
  }

  return std::string(path);
}

std::vector<std::string> RouteTrie::SplitPath(const std::string_view path) {
  std::vector<std::string> segments;
  std::string current;

  for (const char c : path) {
    if (c == '/') {
      if (!current.empty()) {
        segments.push_back(std::move(current));
        current.clear();
      }
    } else {
      current.push_back(c);
    }
  }

  if (!current.empty()) {
    segments.push_back(std::move(current));
  }

  return segments;
}

bool RouteTrie::ParseParamSegment(const std::string& segment, std::string& name,
                                  std::optional<std::regex>& pattern,
                                  PathParamType& type) {
  if (segment.size() < 3 || segment.front() != '{' || segment.back() != '}') {
    return false;
  }

  const std::string content = segment.substr(1, segment.size() - 2);
  const auto colon = content.find(':');

  if (colon == std::string::npos) {
    name = content;
    pattern.reset();
    type = PathParamType::String;
    return true;
  }

  name = content.substr(0, colon);
  const auto spec = std::string_view(content).substr(colon + 1);

  // typed syntax: {id:int} etc.
  if (const auto* info = FindPathParamType(spec)) {
    pattern.emplace("^" + std::string(info->regex) + "$");
    type = info->type;
    return true;
  }

  // legacy regex: {id:\d+} etc.
  pattern.emplace("^" + std::string(spec) + "$");
  type = PathParamType::String;
  return true;
}

void RouteTrie::AddPath(const std::vector<std::string>& segments, const http::verb method,
                        Handler handler) {
  TrieNode* current = root_.get();

  for (const auto& segment : segments) {
    std::optional<std::regex> param_pattern;
    auto param_type{PathParamType::String};

    if (std::string param_name;
        ParseParamSegment(segment, param_name, param_pattern, param_type)) {
      if (!current->param_child) {
        current->param_child = ParamEdge{param_name, param_pattern, param_type,
                                         std::make_unique<TrieNode>()};
      } else {
        if (current->param_child->name != param_name) {
          throw std::logic_error("Conflicting parameter names at same path level");
        }
      }
      current = current->param_child->child.get();
    } else {
      auto& child = current->static_children[segment];
      if (!child) {
        child = std::make_unique<TrieNode>();
      }
      current = child.get();
    }
  }

  current->handlers[method] = std::move(handler);
}

void RouteTrie::AddRoute(const http::verb method, const std::string_view path,
                         Handler handler) {
  const auto normalized = NormalizePath(path);
  const auto segments = SplitPath(normalized);

  AddPath(segments, method, std::move(handler));

  std::cout << "Added route: " << http::to_string(method) << " /" << normalized
            << std::endl;
}

std::pair<const RouteTrie::TrieNode*, std::unordered_map<std::string, std::string>>
RouteTrie::FindPath(const std::vector<std::string>& segments) const {
  const TrieNode* current = root_.get();
  std::unordered_map<std::string, std::string> params;

  for (const auto& segment : segments) {
    if (auto it = current->static_children.find(segment);
        it != current->static_children.end()) {
      current = it->second.get();
      continue;
    }

    if (current->param_child) {
      const auto& edge = *current->param_child;
      if (edge.pattern && !std::regex_match(segment, *edge.pattern)) {
        return {nullptr, {}};
      }
      params[edge.name] = segment;
      current = edge.child.get();
      continue;
    }

    return {nullptr, {}};
  }

  return {current, std::move(params)};
}

std::pair<RouteTrie::Handler, std::unordered_map<std::string, std::string>>
RouteTrie::FindRoute(const http::verb method, const std::string_view path) const {
  const auto normalized = NormalizePath(path);
  const auto segments = SplitPath(normalized);

  auto [node, params] = FindPath(segments);
  if (!node) {
    return {nullptr, {}};
  }

  if (const auto it = node->handlers.find(method); it != node->handlers.end()) {
    return {it->second, std::move(params)};
  }

  return {nullptr, {}};
}

std::vector<std::string> RouteTrie::GetAllRoutes() const {
  std::vector<std::string> routes;

  std::function<void(const TrieNode*, std::string)> walk = [&](const TrieNode* node,
                                                               const std::string& path) {
    for (const auto verb : node->handlers | std::views::keys) {
      routes.push_back(std::string(http::to_string(verb)) + " /" + path);
    }

    for (const auto& [seg, child] : node->static_children) {
      walk(child.get(), path.empty() ? seg : path + "/" + seg);
    }

    if (node->param_child) {
      const auto& edge = *node->param_child;
      const std::string seg = "{" + edge.name + "}";
      walk(edge.child.get(), path.empty() ? seg : path + "/" + seg);
    }
  };

  walk(root_.get(), "");
  return routes;
}
