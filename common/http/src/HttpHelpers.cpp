#include "HttpHelpers.hpp"

Response json_response(const Request& req, http::status status,
                       const nlohmann::json& body) {
  Response res{status, req.version()};
  res.set(http::field::content_type, "application/json");
  res.keep_alive(req.keep_alive());
  res.body() = body.dump();
  res.prepare_payload();
  return res;
}
