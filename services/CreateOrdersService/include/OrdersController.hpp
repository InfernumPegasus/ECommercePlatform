#pragma once

#include <array>
#include <string_view>

#include "IController.hpp"

class OrdersController : public IController<OrdersController> {
 public:
  static constexpr std::string_view BasePath() { return "/orders"; }

  struct RouteDesc {
    http::verb method;
    std::string_view path;
    Response (OrdersController::*handler)(const RequestContext&) const;
  };

  static constexpr std::array<RouteDesc, 7> Routes() {
    return {{
        {http::verb::get, "", &OrdersController::List},
        {http::verb::post, "/remove_all", &OrdersController::RemoveAll},
        {http::verb::get, "/{order_id:\\d+}", &OrdersController::GetById},
        {http::verb::put, "/{order_id:\\d+}", &OrdersController::Update},
        {http::verb::delete_, "/{order_id:\\d+}", &OrdersController::Delete},
        {http::verb::get, "/{order_id:\\d+}/name", &OrdersController::GetOrderName},
        {http::verb::put, "/{order_id:\\d+}/name", &OrdersController::UpdateOrderName},
    }};
  }

  Response List(const RequestContext& ctx) const;
  Response RemoveAll(const RequestContext& ctx) const;
  Response GetById(const RequestContext& ctx) const;
  Response Update(const RequestContext& ctx) const;
  Response Delete(const RequestContext& ctx) const;
  Response GetOrderName(const RequestContext& ctx) const;
  Response UpdateOrderName(const RequestContext& ctx) const;
};
