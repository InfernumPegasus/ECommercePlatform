#pragma once

#include <array>
#include <string_view>
#include <unordered_map>

#include "IController.hpp"
#include "Router.hpp"

class OrdersController : public IController<OrdersController> {
 public:
  static constexpr std::string_view BasePath() { return "/orders"; }

  // Маршруты с параметрами
  // Изменим порядок в RoutesWithParams():
  static constexpr std::array<RouteWithParams, 6> RoutesWithParams() {
    return {
        {{http::verb::get, "/{order_id:\\d+}/name", &OrdersController::GetOrderName},
         {http::verb::put, "/{order_id:\\d+}/name", &OrdersController::UpdateOrderName},
         {http::verb::get, "/{order_id:\\d+}", &OrdersController::GetById},
         {http::verb::put, "/{order_id:\\d+}", &OrdersController::Update},
         {http::verb::delete_, "/{order_id:\\d+}", &OrdersController::Delete}}};
  };

  // Простые маршруты (без параметров) - регистрируем ПЕРВЫМИ
  static constexpr std::array<SimpleRoute, 2> Routes() {
    return {{{http::verb::get, "", &OrdersController::List},
             {http::verb::post, "/remove_all", &OrdersController::RemoveAll}}};
  };

  // Методы с параметрами
  Response GetById(const Request&,
                   const std::unordered_map<std::string, std::string>&) const;
  Response Update(const Request&,
                  const std::unordered_map<std::string, std::string>&) const;
  Response Delete(const Request&,
                  const std::unordered_map<std::string, std::string>&) const;
  Response GetOrderName(const Request&,
                        const std::unordered_map<std::string, std::string>&) const;
  Response UpdateOrderName(const Request&,
                           const std::unordered_map<std::string, std::string>&) const;

  // Методы без параметров
  Response List(const Request&) const;
  Response RemoveAll(const Request&) const;
};
