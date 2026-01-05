#pragma once

#include <array>
#include <string_view>

#include "IController.hpp"

class OrdersController : public IController<OrdersController> {
 public:
  static constexpr std::string_view BasePath() { return "/orders"; }

  static constexpr std::array<RouteDesc, 7> Routes() {
    return {
        {{http::verb::get, "/{order_id:\\d+}", &OrdersController::GetById},
         {http::verb::put, "/{order_id:\\d+}", &OrdersController::Update},
         {http::verb::delete_, "/{order_id:\\d+}", &OrdersController::Delete},
         {http::verb::get, "/{order_id:\\d+}/name", &OrdersController::GetOrderName},
         {http::verb::put, "/{order_id:\\d+}/name", &OrdersController::UpdateOrderName},
         {http::verb::get, "", &OrdersController::List},
         {http::verb::post, "/remove_all", &OrdersController::RemoveAll}}};
  }

  Response List(const RequestContext&) const;
  Response RemoveAll(const RequestContext&) const;
  Response GetById(const RequestContext&) const;
  Response Update(const RequestContext&) const;
  Response Delete(const RequestContext&) const;
  Response GetOrderName(const RequestContext&) const;
  Response UpdateOrderName(const RequestContext&) const;
};
