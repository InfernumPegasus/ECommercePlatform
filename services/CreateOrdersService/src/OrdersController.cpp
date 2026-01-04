#include "OrdersController.hpp"

#include <iostream>
#include <nlohmann/json.hpp>

#include "HttpHelpers.hpp"

Response OrdersController::List(const Request& req) const {
  std::cout << "OrdersController::List called\n";
  return JsonResponse(req, http::status::ok, {{"orders", nlohmann::json::array()}});
}

Response OrdersController::RemoveAll(const Request& req) const {
  std::cout << "OrdersController::RemoveAll called\n";
  return JsonResponse(req, http::status::ok, {{"removed", true}});
}

Response OrdersController::GetById(
    const Request& req,
    const std::unordered_map<std::string, std::string>& params) const {
  std::cout << "OrdersController::GetById called\n";
  auto it = params.find("order_id");
  if (it == params.end()) {
    std::cout << "ERROR: order_id parameter not found!\n";
    return JsonResponse(req, http::status::bad_request,
                        {{"error", "Missing order_id parameter"}});
  }

  nlohmann::json response = {{"order",
                              {{"id", it->second},
                               {"status", "pending"},
                               {"created_at", "2024-01-01T00:00:00Z"}}}};

  return JsonResponse(req, http::status::ok, response);
}

Response OrdersController::Update(
    const Request& req,
    const std::unordered_map<std::string, std::string>& params) const {
  std::cout << "OrdersController::Update called\n";
  auto it = params.find("order_id");
  if (it == params.end()) {
    return JsonResponse(req, http::status::bad_request,
                        {{"error", "Missing order_id parameter"}});
  }

  nlohmann::json response = {
      {"updated", true}, {"id", it->second}, {"message", "Order updated successfully"}};

  return JsonResponse(req, http::status::ok, response);
}

Response OrdersController::Delete(
    const Request& req,
    const std::unordered_map<std::string, std::string>& params) const {
  std::cout << "OrdersController::Delete called\n";
  auto it = params.find("order_id");
  if (it == params.end()) {
    return JsonResponse(req, http::status::bad_request,
                        {{"error", "Missing order_id parameter"}});
  }

  nlohmann::json response = {{"deleted", true}, {"id", it->second}};

  return JsonResponse(req, http::status::ok, response);
}

Response OrdersController::GetOrderName(
    const Request& req,
    const std::unordered_map<std::string, std::string>& params) const {
  std::cout << "OrdersController::GetOrderName called\n";
  auto it = params.find("order_id");
  if (it == params.end()) {
    return JsonResponse(req, http::status::bad_request,
                        {{"error", "Missing order_id parameter"}});
  }

  nlohmann::json response = {{"order_id", it->second},
                             {"name", "Order #" + it->second},
                             {"customer_name", "John Doe"}};

  return JsonResponse(req, http::status::ok, response);
}

Response OrdersController::UpdateOrderName(
    const Request& req,
    const std::unordered_map<std::string, std::string>& params) const {
  std::cout << "OrdersController::UpdateOrderName called\n";
  auto it = params.find("order_id");
  if (it == params.end()) {
    return JsonResponse(req, http::status::bad_request,
                        {{"error", "Missing order_id parameter"}});
  }

  // Парсим тело запроса
  nlohmann::json body;
  try {
    body = nlohmann::json::parse(req.body());
  } catch (const nlohmann::json::exception& e) {
    return JsonResponse(req, http::status::bad_request, {{"error", "Invalid JSON"}});
  }

  std::string new_name = body.value("name", "");
  if (new_name.empty()) {
    return JsonResponse(req, http::status::bad_request, {{"error", "Name is required"}});
  }

  nlohmann::json response = {{"updated", true},
                             {"order_id", it->second},
                             {"new_name", new_name},
                             {"message", "Order name updated successfully"}};

  return JsonResponse(req, http::status::ok, response);
}
