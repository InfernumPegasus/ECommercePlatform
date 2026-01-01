#include "Router.hpp"

void Router::add(http::verb method, std::string path, Handler handler) {
  routes_.emplace(std::make_pair(method, std::move(path)), std::move(handler));
}

Response Router::route(const Request& req) const {
  auto key = std::make_pair(req.method(), std::string(req.target()));
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
