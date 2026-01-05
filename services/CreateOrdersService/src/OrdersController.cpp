#include "OrdersController.hpp"

#include <iostream>
#include <nlohmann/json.hpp>

#include "HttpHelpers.hpp"

Response OrdersController::List(const RequestContext& ctx) const {
  int limit = ctx.query.get<int>("limit").value_or(20);
  int offset = ctx.query.get<int>("offset").value_or(0);
  std::string_view offset_strv = ctx.query.get<std::string_view>("offset").value_or("0");
  auto offset_str = ctx.query.get<std::string>("offset").value_or("0");
  auto offset_bool = ctx.query.get<bool>("offset").value_or(false);

  nlohmann::json response = {
      {"limit", limit}, {"offset", offset}, {"orders", nlohmann::json::array()}};

  return JsonResponse(ctx.request, http::status::ok, response);
}

Response OrdersController::RemoveAll(const RequestContext& ctx) const {
  return JsonResponse(ctx.request, http::status::ok, {{"removed", true}});
}

Response OrdersController::GetById(const RequestContext& ctx) const {
  int order_id = ctx.path.required<int>("order_id");

  return JsonResponse(ctx.request, http::status::ok,
                      {{"order_id", order_id}, {"status", "pending"}});
}

Response OrdersController::Update(const RequestContext& ctx) const {
  int order_id = ctx.path.required<int>("order_id");

  return JsonResponse(ctx.request, http::status::ok,
                      {{"updated", true}, {"order_id", order_id}});
}

Response OrdersController::Delete(const RequestContext& ctx) const {
  int order_id = ctx.path.required<int>("order_id");

  return JsonResponse(ctx.request, http::status::ok,
                      {{"deleted", true}, {"order_id", order_id}});
}

Response OrdersController::GetOrderName(const RequestContext& ctx) const {
  int order_id = ctx.path.required<int>("order_id");

  bool random = ctx.query.get<bool>("random").value_or(false);
  std::cout << "random: " << std::boolalpha << random << std::endl;

  return JsonResponse(ctx.request, http::status::ok,
                      {{"order_id", order_id},
                       {"name", "Order #" + std::to_string(order_id)},
                       {"random", random}});
}

Response OrdersController::UpdateOrderName(const RequestContext& ctx) const {
  int order_id = ctx.path.required<int>("order_id");

  auto body = nlohmann::json::parse(ctx.request.body(), nullptr, false);
  if (!body.is_object() || !body.contains("name") || !body["name"].is_string()) {
    return JsonResponse(ctx.request, http::status::bad_request,
                        {{"error", "field 'name' is required and must be string"}});
  }

  return JsonResponse(
      ctx.request, http::status::ok,
      {{"updated", true}, {"order_id", order_id}, {"new_name", body["name"]}});
}
