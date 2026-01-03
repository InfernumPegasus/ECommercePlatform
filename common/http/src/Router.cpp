#include "Router.hpp"

#include <boost/url.hpp>
#include <iostream>
#include <regex>

Response Router::Route(const Request& req) const {
  const auto parsed = boost::urls::parse_origin_form(req.target());
  if (!parsed) {
    Response res{http::status::bad_request, req.version()};
    res.set(http::field::content_type, "text/plain");
    res.body() = "Invalid request target";
    res.prepare_payload();
    return res;
  }

  std::string path = parsed->path();

  // Отладочная информация
  std::cout << "Routing request: " << http::to_string(req.method()) << " " << path
            << "\n";

  // Нормализуем путь
  if (path.size() > 1 && path.ends_with('/')) {
    path.pop_back();
  }

  // Для корневого пути "/" оставляем как есть
  if (path == "/") {
    // Это уже нормализованный корневой путь
  }

  // Ищем подходящий маршрут
  for (const auto& route : routes_) {
    if (route.method != req.method()) {
      continue;
    }

    std::cout << "  Trying pattern: " << route.pattern << "\n";
    std::regex pattern(route.pattern);
    std::smatch matches;

    if (std::regex_match(path, matches, pattern)) {
      std::cout << "  MATCH FOUND! Number of matches: " << matches.size() << "\n";
      for (size_t i = 0; i < matches.size(); ++i) {
        std::cout << "    Match[" << i << "]: " << matches[i].str() << "\n";
      }
      std::cout << "  Parameter names count: " << route.param_names.size() << "\n";

      // Нашли совпадение, вызываем хендлер
      return route.handler(req, matches);
    }
  }

  std::cout << "  NO MATCH FOUND\n";

  // Маршрут не найден
  Response res{http::status::not_found, req.version()};
  res.set(http::field::content_type, "text/plain");
  res.body() = "Route not found";
  res.prepare_payload();
  return res;
}
