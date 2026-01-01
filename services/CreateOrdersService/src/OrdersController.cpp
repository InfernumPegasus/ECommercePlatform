#include "OrdersController.hpp"

#include "HttpHelpers.hpp"

// TODO constexpr map
Response OrdersController::dispatch(const RouteDesc& route, const Request& req) {
  using enum http::verb;

  if (route.method == get && route.path == "/") {
    return this->list(req);
  }
  if (route.method == post && route.path == "/") {
    return this->create(req);
  }
  if (route.method == get && route.path == "/{id}") {
    return this->get(req);
  }
  if (route.method == get && route.path == "/exactly_42") {
    return this->get(req);
  }

  return json_response(req, http::status::not_found, {{"error", "route not handled"}});
}

Response OrdersController::list(const Request& req) {
  return json_response(req, http::status::ok, {{"orders", nlohmann::json::array()}});
}

Response OrdersController::create(const Request& req) {
  auto body = nlohmann::json::parse(req.body());
  return json_response(req, http::status::created,
                       {{"created", true}, {"payload", body}});
}

Response OrdersController::get(const Request& req) {
  return json_response(req, http::status::ok, {{"id", "42"}});
}
