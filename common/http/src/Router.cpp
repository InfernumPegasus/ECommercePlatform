#include "Router.hpp"

#include <boost/url.hpp>
#include <iostream>

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
            << std::endl;

  auto [handler, params] = trie_.FindRoute(req.method(), path);

  if (handler) {
    std::cout << "  Route found with " << params.size() << " parameters" << std::endl;
    for (const auto& [key, value] : params) {
      std::cout << "    " << key << " = " << value << std::endl;
    }
    return handler(req, params);
  }

  std::cout << "  Route not found" << std::endl;

  // Маршрут не найден
  Response res{http::status::not_found, req.version()};
  res.set(http::field::content_type, "text/plain");
  res.body() = "Route not found";
  res.prepare_payload();
  return res;
}

void Router::PrintAllRoutes() const {
  const auto routes = trie_.GetAllRoutes();
  std::cout << "All registered routes:" << std::endl;
  for (const auto& route : routes) {
    std::cout << "  " << route << std::endl;
  }
}
