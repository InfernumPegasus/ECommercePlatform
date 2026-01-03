#include "OrdersController.hpp"

#include "HttpHelpers.hpp"

Response OrdersController::List(const Request& req) const {
  return JsonResponse(req, http::status::ok, {{"orders", nlohmann::json::array()}});
}

Response OrdersController::RemoveAll(const Request& req) const {
  return JsonResponse(req, http::status::ok, {{"removed", true}});
}
