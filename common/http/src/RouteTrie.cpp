#include "RouteTrie.hpp"

#include <algorithm>
#include <iostream>
#include <ranges>

#include "PathParamType.hpp"

RouteTrie::RouteTrie() : root_(std::make_unique<TrieNode>()) {}

std::string_view RouteTrie::NormalizePathView(std::string_view path) {
  if (!path.empty() && path.front() == '/') {
    path.remove_prefix(1);
  }
  if (!path.empty() && path.back() == '/') {
    path.remove_suffix(1);
  }

  return path;
}

std::string RouteTrie::NormalizePathOwned(const std::string_view path) {
  return std::string(NormalizePathView(path));
}

std::vector<std::string_view> RouteTrie::SplitPathView(const std::string_view path) {
  std::vector<std::string_view> segments;
  if (!path.empty()) {
    segments.reserve(
        static_cast<std::size_t>(std::count(path.begin(), path.end(), '/') + 1));
  }

  std::size_t segment_start = 0;
  for (std::size_t i = 0; i < path.size(); ++i) {
    if (path[i] != '/') {
      continue;
    }
    if (i > segment_start) {
      segments.push_back(path.substr(segment_start, i - segment_start));
    }
    segment_start = i + 1;
  }

  if (segment_start < path.size()) {
    segments.push_back(path.substr(segment_start));
  }

  return segments;
}

std::vector<std::string> RouteTrie::SplitPathOwned(const std::string_view path) {
  std::vector<std::string> segments;
  auto view_segments = SplitPathView(path);
  segments.reserve(view_segments.size());
  for (const auto segment : view_segments) {
    segments.emplace_back(segment);
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
        current = it->second.get();
      } else {
        for (const auto& [existing_key, _] : current->param_children) {
          if (existing_key.type == key.type) {
            throw std::runtime_error(
                "Ambiguous parameter routes with same matcher at: {" + existing_key.name +
                "} and {" + key.name + "}");
          }
        }

        auto new_node = std::make_unique<TrieNode>();
        auto* new_node_ptr = new_node.get();

        current->param_children.emplace(std::move(key), std::move(new_node));
        current = new_node_ptr;
      }
    } else {
      auto& child = current->static_children[segment];
      if (!child) {
        child = std::make_unique<TrieNode>();
      }
      current = child.get();
    }
  }

  if (current->handlers.contains(method)) {
    throw std::runtime_error("Duplicate route registration for method/path");
  }

  current->handlers.emplace(method, std::move(handler));
}

void RouteTrie::AddRoute(const http::verb method, const std::string_view path,
                         Handler handler) {
  const auto normalized = NormalizePathOwned(path);
  const auto segments = SplitPathOwned(normalized);

  AddPath(segments, method, std::move(handler));

  std::cout << "Added route: " << http::to_string(method) << " /" << normalized
            << std::endl;
}

std::pair<const RouteTrie::TrieNode*, std::unordered_map<std::string, std::string>>
RouteTrie::FindPath(const std::vector<std::string_view>& segments) const {
  const TrieNode* current = root_.get();
  std::unordered_map<std::string, std::string> params;
  params.reserve(segments.size());

  for (const auto& segment : segments) {
    if (auto it = current->static_children.find(segment);
        it != current->static_children.end()) {
      current = it->second.get();
      continue;
    }

    const std::unique_ptr<TrieNode>* best_child = nullptr;
    const ParamKey* best_key = nullptr;

    for (const auto& [key, child] : current->param_children) {
      if (!key.type->matcher(segment)) {
        continue;
      }

      best_child = &child;
      best_key = &key;
      break;
    }

    if (!best_child || !best_key) {
      return {nullptr, {}};
    }

    params[best_key->name] = std::string(segment);
    current = best_child->get();
  }

  return {current, std::move(params)};
}

std::pair<RouteTrie::Handler, std::unordered_map<std::string, std::string>>
RouteTrie::FindRoute(const http::verb method, const std::string_view path) const {
  const auto normalized = NormalizePathView(path);
  const auto segments = SplitPathView(normalized);

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

  auto walk = [&routes](this auto&& self, const TrieNode* node,
                        const std::string& path) -> void {
    for (const auto verb : node->handlers | std::views::keys) {
      routes.push_back(std::string(http::to_string(verb)) + " /" + path);
    }

    for (const auto& [seg, child] : node->static_children) {
      self(child.get(), path.empty() ? seg : path + "/" + seg);
    }

    for (const auto& [key, child] : node->param_children) {
      const std::string seg = "{" + key.name + ":" + std::string(key.type->name) + "}";
      self(child.get(), path.empty() ? seg : path + "/" + seg);
    }
  };

  walk(root_.get(), "");
  return routes;
}
