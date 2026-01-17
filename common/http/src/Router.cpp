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

  std::cout << "Routing request: " << http::to_string(req.method()) << " "
            << parsed->path() << std::endl;

  GeneralParams query;
  for (auto p : parsed->params()) {
    query.emplace(p.key, p.value);
  }

  auto [handler, path_params] = trie_.FindRoute(req.method(), parsed->path());
  if (!handler) {
    Response res{http::status::not_found, req.version()};
    res.set(http::field::content_type, "text/plain");
    res.body() = "Route not found";
    res.prepare_payload();
    return res;
  }

  RequestContext ctx(req, std::move(path_params), std::move(query));
  return handler(ctx);
}

void Router::PrintAllRoutes() const {
  const auto routes = trie_.GetAllRoutes();
  std::cout << "All registered routes:" << std::endl;
  for (const auto& route : routes) {
    std::cout << "  " << route << std::endl;
  }
}
