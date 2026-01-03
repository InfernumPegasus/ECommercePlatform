#pragma once

#include <array>
#include <string_view>

#include "IController.hpp"
#include "Router.hpp"

class OrdersController : public IController<OrdersController> {
 public:
  static constexpr std::string_view BasePath() { return "/orders"; }

  static constexpr std::array<Route, 2> Routes() {
    return {{{http::verb::get, "/", &OrdersController::List},
             {http::verb::post, "/remove_all", &OrdersController::RemoveAll}}};
  };

  Response List(const Request&) const;
  Response RemoveAll(const Request&) const;
};
