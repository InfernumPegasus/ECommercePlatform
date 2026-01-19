#include "RouteTrie.hpp"

#include <iostream>
#include <ranges>

#include "PathParamType.hpp"

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
                                  const PathParamTypeDescriptor*& type) {
  if (segment.size() < 3 || segment.front() != '{' || segment.back() != '}') {
    return false;
  }

  const std::string content = segment.substr(1, segment.size() - 2);
  const size_t colon = content.find(':');

  if (colon == std::string::npos) {
    name = content;
    type = PathParamRegistry::Default();
    return true;
  }

  name = content.substr(0, colon);
  const auto spec = std::string_view(content).substr(colon + 1);

  if (const auto* found = PathParamRegistry::FindByName(spec)) {
    type = found;
    return true;
  }

  throw std::runtime_error("Unknown path param type: " + std::string(spec));
}

void RouteTrie::AddPath(const std::vector<std::string>& segments, const http::verb method,
                        Handler handler) {
  TrieNode* current = root_.get();

  for (const auto& segment : segments) {
    const PathParamTypeDescriptor* param_type = nullptr;

    if (std::string param_name; ParseParamSegment(segment, param_name, param_type)) {
      ParamKey key{std::move(param_name), param_type};

      if (auto it = current->param_children.find(key);
          it != current->param_children.end()) {
        current = it->second.child.get();
      } else {
        ParamEdge new_edge;
        new_edge.child = std::make_unique<TrieNode>();

        const auto [iter, _] = current->param_children.insert(
            std::make_pair(std::move(key), std::move(new_edge)));
        current = iter->second.child.get();
      }
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

    const ParamEdge* best_edge = nullptr;
    const ParamKey* best_key = nullptr;

    for (const auto& [key, edge] : current->param_children) {
      if (!key.type->matcher(segment)) {
        continue;
      }

      best_edge = &edge;
      best_key = &key;
      break;
    }

    if (!best_edge || !best_key) {
      return {nullptr, {}};
    }

    params[best_key->name] = segment;
    current = best_edge->child.get();
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

    for (const auto& [key, edge] : node->param_children) {
      const std::string seg = "{" + key.name + ":" + std::string(key.type->name) + "}";

      walk(edge.child.get(), path.empty() ? seg : path + "/" + seg);
    }
  };

  walk(root_.get(), "");
  return routes;
}
