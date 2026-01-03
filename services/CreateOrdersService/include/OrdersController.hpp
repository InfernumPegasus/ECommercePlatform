#pragma once

#include <array>
#include <string_view>

#include "IController.hpp"

class OrdersController : public IController<OrdersController> {
 public:
  static constexpr std::string_view BasePath() { return "/orders"; }

  // TODO add constexpr map
  static constexpr std::array<RouteDesc, 3> Routes() {
    return {{{http::verb::get, "/"},
             {http::verb::post, "/"},
             {http::verb::get, "/exactly_42"}}};
  }

  Response Dispatch(const RouteDesc& route, const Request& req);

 private:
  Response list(const Request& req);
  Response create(const Request& req);
  Response get(const Request& req);
};
