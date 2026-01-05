#pragma once

#include <boost/beast/http.hpp>
#include <functional>
#include <memory>
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

  // Добавить маршрут
  void AddRoute(http::verb method, const std::string& path, Handler handler);

  // Найти маршрут
  std::pair<Handler, std::unordered_map<std::string, std::string>> FindRoute(
      http::verb method, const std::string& path) const;

  // Получить все маршруты (для отладки)
  std::vector<std::string> GetAllRoutes() const;

 private:
  struct TrieNode {
    // Статические дочерние узлы (точное совпадение)
    std::unordered_map<std::string, std::unique_ptr<TrieNode>> static_children;

    // Динамические параметры (именованные)
    std::unordered_map<std::string, std::pair<std::string, std::unique_ptr<TrieNode>>>
        param_children;

    // Обработчики для каждого HTTP метода
    std::unordered_map<http::verb, Handler> handlers;

    // Является ли этот узел параметрическим
    std::string param_name;
    bool is_param = false;

    TrieNode() = default;
  };

  std::unique_ptr<TrieNode> root_;

  // Разбить путь на сегменты
  static std::vector<std::string> SplitPath(const std::string& path);

  // Проверить, является ли сегмент параметром
  static bool IsParamSegment(const std::string& segment, std::string& param_name,
                             std::string& param_pattern);

  // Добавить путь в trie
  void AddPath(const std::vector<std::string>& segments, http::verb method,
               Handler handler);

  // Найти путь в trie
  std::pair<TrieNode*, std::unordered_map<std::string, std::string>> FindPath(
      const std::vector<std::string>& segments) const;
};
