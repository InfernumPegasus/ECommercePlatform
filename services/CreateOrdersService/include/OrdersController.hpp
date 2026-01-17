#pragma once

#include <array>
#include <string_view>

#include "IController.hpp"
#include "RouteDSL.hpp"

class OrdersController : public IController<OrdersController> {
 public:
  static constexpr std::string_view BasePath() { return "/orders"; }

  static constexpr auto Routes() {
    using R = route_dsl::RouteBuilder<OrdersController>;

    return R::Routes(R::GET("", &OrdersController::List),
                     R::POST("/remove_all", &OrdersController::RemoveAll),
                     R::GET("/{order_id:int}", &OrdersController::GetById),
                     R::PUT("/{order_id:int}", &OrdersController::Update),
                     R::DEL("/{order_id:int}", &OrdersController::Delete),
                     R::GET("/{order_id:int}/name", &OrdersController::GetOrderName),
                     R::PUT("/{order_id:int}/name", &OrdersController::UpdateOrderName),
                     R::PUT("/{order_id:int}/name/{random_string:string}",
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
