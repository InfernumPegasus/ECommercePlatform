#include "RouteTrie.hpp"

#include <iostream>
#include <ranges>
#include <regex>

RouteTrie::RouteTrie() : root_(std::make_unique<TrieNode>()) {}

std::vector<std::string> RouteTrie::SplitPath(const std::string& path) {
  std::vector<std::string> segments;
  std::string segment;

  for (const char c : path) {
    if (c == '/') {
      if (!segment.empty()) {
        segments.push_back(segment);
        segment.clear();
      }
    } else {
      segment.push_back(c);
    }
  }

  if (!segment.empty()) {
    segments.push_back(segment);
  }

  return segments;
}

bool RouteTrie::IsParamSegment(const std::string& segment, std::string& param_name,
                               std::string& param_pattern) {
  if (segment.size() < 3 || segment.front() != '{' || segment.back() != '}') {
    return false;
  }

  std::string content = segment.substr(1, segment.size() - 2);

  // Проверяем есть ли паттерн: {name:pattern}
  size_t colon_pos = content.find(':');
  if (colon_pos != std::string::npos) {
    param_name = content.substr(0, colon_pos);
    param_pattern = content.substr(colon_pos + 1);
  } else {
    param_name = content;
    param_pattern = "";  // Без паттерна
  }

  return true;
}

void RouteTrie::AddPath(const std::vector<std::string>& segments, http::verb method,
                        Handler handler) {
  TrieNode* current = root_.get();

  for (const auto& segment : segments) {
    std::string param_name, param_pattern;

    if (IsParamSegment(segment, param_name, param_pattern)) {
      // Параметрический сегмент
      auto& param_child = current->param_children[param_pattern];
      if (!param_child.second) {
        param_child.second = std::make_unique<TrieNode>();
        param_child.second->is_param = true;
        param_child.second->param_name = param_name;
      }
      current = param_child.second.get();
      param_child.first = param_name;
    } else {
      // Статический сегмент
      auto& child = current->static_children[segment];
      if (!child) {
        child = std::make_unique<TrieNode>();
      }
      current = child.get();
    }
  }

  current->handlers[method] = handler;
}

void RouteTrie::AddRoute(http::verb method, const std::string& path, Handler handler) {
  std::string normalized_path = path;

  // Убрать начальный и конечный слэши
  if (!normalized_path.empty() && normalized_path.front() == '/') {
    normalized_path = normalized_path.substr(1);
  }
  if (!normalized_path.empty() && normalized_path.back() == '/') {
    normalized_path.pop_back();
  }

  const auto segments = SplitPath(normalized_path);
  AddPath(segments, method, handler);

  // Для отладки
  std::cout << "Added route: " << http::to_string(method) << " " << path << std::endl;
}

std::pair<RouteTrie::TrieNode*, std::unordered_map<std::string, std::string>>
RouteTrie::FindPath(const std::vector<std::string>& segments) const {
  TrieNode* current = root_.get();
  std::unordered_map<std::string, std::string> params;

  for (const auto& segment : segments) {
    bool found = false;

    // Сначала проверяем статические дочерние узлы
    auto static_it = current->static_children.find(segment);
    if (static_it != current->static_children.end()) {
      current = static_it->second.get();
      found = true;
    }
    // Затем проверяем параметрические
    else if (!current->param_children.empty()) {
      // Пробуем все параметрические паттерны
      for (auto& [pattern, param_info] : current->param_children) {
        auto& [param_name, child_node] = param_info;

        // Если есть паттерн, проверяем соответствие
        if (!pattern.empty()) {
          std::regex pattern_regex("^" + pattern + "$");
          if (!std::regex_match(segment, pattern_regex)) {
            continue;  // Не соответствует паттерну
          }
        }

        // Сохраняем значение параметра
        params[param_name] = segment;
        current = child_node.get();
        found = true;
        break;  // Берем первый подходящий параметрический узел
      }
    }

    if (!found) {
      return {nullptr, {}};
    }
  }

  return {current, std::move(params)};
}

std::pair<RouteTrie::Handler, std::unordered_map<std::string, std::string>>
RouteTrie::FindRoute(http::verb method, const std::string& path) const {
  std::string normalized_path = path;

  // Нормализация пути
  if (!normalized_path.empty() && normalized_path.front() == '/') {
    normalized_path = normalized_path.substr(1);
  }
  if (!normalized_path.empty() && normalized_path.back() == '/') {
    normalized_path.pop_back();
  }

  const auto segments = SplitPath(normalized_path);
  auto [node, params] = FindPath(segments);

  if (node && node->handlers.contains(method)) {
    return {node->handlers.at(method), std::move(params)};
  }

  return {nullptr, {}};
}

std::vector<std::string> RouteTrie::GetAllRoutes() const {
  std::vector<std::string> routes;

  // Рекурсивный обход trie
  std::function<void(TrieNode*, std::string)> traverse =
      [&](TrieNode* node, const std::string& current_path) {
        if (!node) return;

        // Добавляем маршруты этого узла
        for (const auto& method : node->handlers | std::views::keys) {
          routes.push_back(std::string(http::to_string(method)) + " /" + current_path);
        }

        // Статические дочерние узлы
        for (const auto& [segment, child] : node->static_children) {
          traverse(child.get(),
                   current_path + (current_path.empty() ? "" : "/") + segment);
        }

        // Параметрические дочерние узлы
        for (const auto& [pattern, param_info] : node->param_children) {
          const auto& [param_name, child] = param_info;
          std::string param_segment = "{" + param_name;
          if (!pattern.empty()) {
            param_segment += ":" + pattern;
          }
          param_segment += "}";
          traverse(child.get(),
                   current_path + (current_path.empty() ? "" : "/") + param_segment);
        }
      };

  traverse(root_.get(), "");
  return routes;
}
