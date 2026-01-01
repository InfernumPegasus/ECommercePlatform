#pragma once

#include <array>
#include <string_view>

#include "IController.hpp"

class OrdersController : public IController<OrdersController> {
 public:
  static constexpr std::string_view base_path() { return "/orders"; }

  static constexpr std::array<RouteDesc, 4> routes() {
    return {{{http::verb::get, "/"},
             {http::verb::post, "/"},
             {http::verb::get, "/{id}"},
             {http::verb::get, "/exactly_42"}}};
  }

  Response dispatch(const RouteDesc& route, const Request& req);

 private:
  Response list(const Request& req);
  Response create(const Request& req);
  Response get(const Request& req);
};
