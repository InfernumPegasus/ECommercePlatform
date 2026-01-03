#include "Router.hpp"

#include <boost/url.hpp>

namespace {
std::string NormalizePath(std::string_view path) {
  if (path.size() > 1 && path.ends_with('/')) {
    path.remove_suffix(1);
  }
  return std::string(path);
}
}  // namespace

void Router::AddRoute(http::verb method, std::string_view path, Handler handler) {
  routes_.emplace(std::make_pair(method, NormalizePath(path)), std::move(handler));
}

Response Router::Route(const Request& req) const {
  auto parsed = boost::urls::parse_origin_form(req.target());
  if (!parsed) {
    Response res{http::status::bad_request, req.version()};
    res.body() = "Invalid request target";
    res.prepare_payload();
    return res;
  }

  const auto path = NormalizePath(parsed->path());
  auto key = std::make_pair(req.method(), path);

  auto it = routes_.find(key);
  if (it == routes_.end()) {
    Response res{http::status::not_found, req.version()};
    res.set(http::field::content_type, "text/plain");
    res.body() = "Route not found";
    res.prepare_payload();
    return res;
  }

  return it->second(req);
}
