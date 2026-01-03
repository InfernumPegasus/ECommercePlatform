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
  static constexpr std::array<RouteWithParams, 3> RoutesWithParams() {
    return {{{http::verb::get, "/{id:\\d+}", &OrdersController::GetById},
             {http::verb::put, "/{id:\\d+}", &OrdersController::Update},
             {http::verb::delete_, "/{id:\\d+}", &OrdersController::Delete}}};
  };

  // Простые маршруты (без параметров) - регистрируем ПЕРВЫМИ
  // Используем "" вместо "/" для корневого пути контроллера
  static constexpr std::array<SimpleRoute, 2> Routes() {
    return {{{http::verb::get, "", &OrdersController::List},  // ИЗМЕНЕНО: "" вместо "/"
             {http::verb::post, "/remove_all", &OrdersController::RemoveAll}}};
  };

  // Методы с параметрами
  Response GetById(const Request&,
                   const std::unordered_map<std::string, std::string>&) const;
  Response Update(const Request&,
                  const std::unordered_map<std::string, std::string>&) const;
  Response Delete(const Request&,
                  const std::unordered_map<std::string, std::string>&) const;

  // Методы без параметров
  Response List(const Request&) const;
  Response RemoveAll(const Request&) const;
};
