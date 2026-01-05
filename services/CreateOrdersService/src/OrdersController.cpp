#include "OrdersController.hpp"

#include <iostream>
#include <nlohmann/json.hpp>

#include "HttpHelpers.hpp"

Response OrdersController::List(const RequestContext& ctx) const {
  auto limit = ctx.query.get("limit").value_or("20");
  auto offset = ctx.query.get("offset").value_or("0");

  nlohmann::json response = {
      {"limit", limit}, {"offset", offset}, {"orders", nlohmann::json::array()}};

  return JsonResponse(ctx.request, http::status::ok, response);
}

Response OrdersController::RemoveAll(const RequestContext& ctx) const {
  return JsonResponse(ctx.request, http::status::ok, {{"removed", true}});
}

Response OrdersController::GetById(const RequestContext& ctx) const {
  const auto& id = ctx.path_params.at("order_id");

  return JsonResponse(ctx.request, http::status::ok, {{"id", id}, {"status", "pending"}});
}

Response OrdersController::Update(const RequestContext& ctx) const {
  const auto& id = ctx.path_params.at("order_id");
  return JsonResponse(ctx.request, http::status::ok, {{"updated", true}, {"id", id}});
}

Response OrdersController::Delete(const RequestContext& ctx) const {
  const auto& id = ctx.path_params.at("order_id");
  return JsonResponse(ctx.request, http::status::ok, {{"deleted", true}, {"id", id}});
}

Response OrdersController::GetOrderName(const RequestContext& ctx) const {
  const auto& id = ctx.path_params.at("order_id");
  return JsonResponse(ctx.request, http::status::ok,
                      {{"order_id", id}, {"name", "Order #" + id}});
}

Response OrdersController::UpdateOrderName(const RequestContext& ctx) const {
  const auto& id = ctx.path_params.at("order_id");

  auto body = nlohmann::json::parse(ctx.request.body(), nullptr, false);
  if (!body.is_object() || !body.contains("name")) {
    return JsonResponse(ctx.request, http::status::bad_request,
                        {{"error", "name is required"}});
  }

  return JsonResponse(ctx.request, http::status::ok,
                      {{"updated", true}, {"order_id", id}, {"new_name", body["name"]}});
}
