#include "Router.hpp"

#include <boost/url.hpp>

Response Router::Route(const Request& req) const {
  const auto parsed = boost::urls::parse_origin_form(req.target());
  if (!parsed) {
    Response res{http::status::bad_request, req.version()};
    res.set(http::field::content_type, "text/plain");
    res.body() = "Invalid request target";
    res.prepare_payload();

    return res;
  }

  const auto path = NormalizePath(parsed->path());
  const auto key = std::make_pair(req.method(), path);

  const auto it = routes_.find(key);
  if (it == routes_.end()) {
    Response res{http::status::not_found, req.version()};
    res.set(http::field::content_type, "text/plain");
    res.body() = "Route not found";
    res.prepare_payload();

    return res;
  }

  return it->second(req);
}
