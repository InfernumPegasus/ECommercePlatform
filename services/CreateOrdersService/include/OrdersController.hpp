#pragma once

#include <array>
#include <string_view>

#include "IController.hpp"
#include "RouteDSL.hpp"

class OrdersController : public IController<OrdersController> {
 public:
  static constexpr auto Routes() {
    return route_dsl::WithBase(
        "/orders", route_dsl::GET("", &OrdersController::List),
        route_dsl::POST("/remove_all", &OrdersController::RemoveAll),
        route_dsl::GET("/{order_id:int}", &OrdersController::GetById),
        route_dsl::PUT("/{order_id:int}", &OrdersController::Update),
        route_dsl::DEL("/{order_id:int}", &OrdersController::Delete),
        route_dsl::GET("/{order_id:int}/name", &OrdersController::GetOrderName),
        route_dsl::PUT("/{order_id:int}/name", &OrdersController::UpdateOrderName),
        route_dsl::PUT("/{order_id:int}/name/{random_string:string}",
                       &OrdersController::TestMethod));
  }

  Response List(const RequestContext& ctx);
  Response RemoveAll(const RequestContext& ctx);
  Response GetById(const RequestContext& ctx);
  Response Update(const RequestContext& ctx);
  Response Delete(const RequestContext& ctx);
  Response GetOrderName(const RequestContext& ctx);
  Response UpdateOrderName(const RequestContext& ctx);
  Response TestMethod(const RequestContext& ctx);
};
