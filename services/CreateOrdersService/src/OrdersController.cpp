#include "OrdersController.hpp"

#include <iostream>
#include <nlohmann/json.hpp>

#include "HttpHelpers.hpp"

Response OrdersController::List(const RequestContext& ctx) const {
  int limit = ctx.GetQueryParameters().TryGet<int>("limit").value_or(20);
  int offset = ctx.GetQueryParameters().TryGet<int>("offset").value_or(0);
  std::string_view offset_strv =
      ctx.GetQueryParameters().TryGet<std::string_view>("offset").value_or("0");
  auto offset_str = ctx.GetQueryParameters().TryGet<std::string>("offset").value_or("0");
  auto offset_bool = ctx.GetQueryParameters().TryGet<bool>("offset").value_or(false);

  nlohmann::json response = {
      {"limit", limit}, {"offset", offset}, {"orders", nlohmann::json::array()}};

  return JsonResponse(ctx.GetRequest(), http::status::ok, response);
}

Response OrdersController::RemoveAll(const RequestContext& ctx) const {
  return JsonResponse(ctx.GetRequest(), http::status::ok, {{"removed", true}});
}

Response OrdersController::GetById(const RequestContext& ctx) const {
  auto order_id = ctx.GetPathParameters().Required<double>("order_id");

  return JsonResponse(ctx.GetRequest(), http::status::ok,
                      {{"order_id", order_id}, {"status", "pending"}});
}

Response OrdersController::Update(const RequestContext& ctx) const {
  int order_id = ctx.GetPathParameters().Required<int>("order_id");

  return JsonResponse(ctx.GetRequest(), http::status::ok,
                      {{"updated", true}, {"order_id", order_id}});
}

Response OrdersController::Delete(const RequestContext& ctx) const {
  int order_id = ctx.GetPathParameters().Required<int>("order_id");

  return JsonResponse(ctx.GetRequest(), http::status::ok,
                      {{"deleted", true}, {"order_id", order_id}});
}

Response OrdersController::GetOrderName(const RequestContext& ctx) const {
  int order_id = ctx.GetPathParameters().Required<int>("order_id");

  bool random = ctx.GetQueryParameters().TryGet<bool>("random").value_or(false);
  std::cout << "random: " << std::boolalpha << random << std::endl;

  return JsonResponse(ctx.GetRequest(), http::status::ok,
                      {{"order_id", order_id},
                       {"name", "Order #" + std::to_string(order_id)},
                       {"random", random}});
}

Response OrdersController::UpdateOrderName(const RequestContext& ctx) const {
  int order_id = ctx.GetPathParameters().Required<int>("order_id");

  auto body = nlohmann::json::parse(ctx.GetRequest().body(), nullptr, false);
  if (!body.is_object() || !body.contains("name") || !body["name"].is_string()) {
    return JsonResponse(ctx.GetRequest(), http::status::bad_request,
                        {{"error", "field 'name' is required and must be string"}});
  }

  return JsonResponse(
      ctx.GetRequest(), http::status::ok,
      {{"updated", true}, {"order_id", order_id}, {"new_name", body["name"]}});
}

Response OrdersController::TestMethod(const RequestContext& ctx) const {
  int order_id = ctx.GetPathParameters().Required<int>("order_id");
  std::string random_string =
      ctx.GetPathParameters().Required<std::string>("random_string");

  return JsonResponse(ctx.GetRequest(), http::status::ok,
                      {{"order_id", order_id}, {"random_string", random_string}});
}
