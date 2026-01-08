#pragma once

#include <array>
#include <string_view>

#include "IController.hpp"

class OrdersController : public IController<OrdersController> {
 public:
  static constexpr std::string_view BasePath() { return "/orders"; }

  static constexpr auto Routes() {
    return std::to_array<RouteDescription>({
        {http::verb::get, "", &OrdersController::List},
        {http::verb::post, "/remove_all", &OrdersController::RemoveAll},
        {http::verb::get, "/{order_id:float}", &OrdersController::GetById},
        {http::verb::put, "/{order_id:int}", &OrdersController::Update},
        {http::verb::delete_, "/{order_id:int}", &OrdersController::Delete},
        {http::verb::get, "/{order_id:int}/name", &OrdersController::GetOrderName},
        {http::verb::put, "/{order_id:int}/name", &OrdersController::UpdateOrderName},
        {http::verb::get, "/{order_id:int}/name/{random_string:string}",
         &OrdersController::TestMethod},
    });
  }

  Response List(const RequestContext& ctx) const;
  Response RemoveAll(const RequestContext& ctx) const;
  Response GetById(const RequestContext& ctx) const;
  Response Update(const RequestContext& ctx) const;
  Response Delete(const RequestContext& ctx) const;
  Response GetOrderName(const RequestContext& ctx) const;
  Response UpdateOrderName(const RequestContext& ctx) const;
  Response TestMethod(const RequestContext& ctx) const;
};
