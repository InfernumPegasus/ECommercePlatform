#pragma once

#include <boost/beast/http.hpp>
#include <functional>
#include <iostream>
#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

#include "HttpTypes.hpp"

struct RouteDesc {
  http::verb method;
  std::string_view path;
};

class Router {
 public:
  // Хендлер с параметрами
  template <typename Controller>
  void AddRoute(http::verb method, const std::string& path,
                Response (Controller::*handler)(
                    const Request&, const std::unordered_map<std::string, std::string>&)
                    const,
                const Controller* instance) {
    auto [pattern, param_names] = ParsePathPattern(path);

    // Отладочный вывод
    std::cout << "Registering route: " << http::to_string(method) << " " << path
              << " -> pattern: " << pattern << "\n";

    routes_.push_back(RouteInfo{
        method, pattern, param_names,
        [handler, instance, param_names](const Request& req,
                                         const std::smatch& matches) -> Response {
          std::unordered_map<std::string, std::string> params;
          for (size_t i = 0; i < param_names.size() && i + 1 < matches.size(); ++i) {
            params[param_names[i]] = matches[i + 1].str();
          }
          return (instance->*handler)(req, params);
        }});
  }

  // Хендлер без параметров
  template <typename Controller>
  void AddRoute(http::verb method, const std::string& path,
                Response (Controller::*handler)(const Request&) const,
                const Controller* instance) {
    auto [pattern, param_names] = ParsePathPattern(path);

    // Отладочный вывод
    std::cout << "Registering route: " << http::to_string(method) << " " << path
              << " -> pattern: " << pattern << "\n";

    routes_.push_back(RouteInfo{
        method, pattern, param_names,
        [handler, instance](const Request& req, const std::smatch&) -> Response {
          return (instance->*handler)(req);
        }});
  }

  Response Route(const Request& req) const;

 private:
  using Handler = std::function<Response(const Request&, const std::smatch&)>;

  struct RouteInfo {
    http::verb method;
    std::string pattern;  // regex pattern
    std::vector<std::string> param_names;
    Handler handler;
  };

  std::vector<RouteInfo> routes_;

  static std::pair<std::string, std::vector<std::string>> ParsePathPattern(
      const std::string& path) {
    std::vector<std::string> param_names;
    std::string result = path;

    // Регулярное выражение для захвата {param} или {param:regex}
    std::regex param_regex(R"(\{([^}:]+)(?::([^}]+))?\})");

    auto begin = std::sregex_iterator(path.begin(), path.end(), param_regex);
    auto end = std::sregex_iterator();

    size_t pos = 0;
    for (auto i = begin; i != end; ++i) {
      std::smatch match = *i;
      std::string param_name = match[1].str();
      std::string param_pattern = match[2].str();
      param_names.push_back(param_name);

      // Используем пользовательский паттерн или стандартный
      std::string replacement =
          param_pattern.empty() ? "([^/]+)" : "(" + param_pattern + ")";

      // Заменяем с учетом предыдущих замен
      size_t match_pos = match.position();
      if (pos > 0) {
        match_pos += pos;
      }

      result.replace(match_pos, match.length(), replacement);
      pos += replacement.length() - match.length();
    }

    // Явно добавляем начало и конец строки для точного совпадения
    result = "^" + result + "$";

    return {result, param_names};
  }
};
