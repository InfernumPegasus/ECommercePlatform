#include "OrdersController.hpp"

#include <iostream>
#include <nlohmann/json.hpp>

#include "HttpHelpers.hpp"

Response OrdersController::List(const Request& req) const {
  return JsonResponse(req, http::status::ok, {{"orders", nlohmann::json::array()}});
}

Response OrdersController::RemoveAll(const Request& req) const {
  return JsonResponse(req, http::status::ok, {{"removed", true}});
}

Response OrdersController::GetById(
    const Request& req,
    const std::unordered_map<std::string, std::string>& params) const {
  std::cout << "GetById called with " << params.size() << " parameters\n";
  for (const auto& [key, value] : params) {
    std::cout << "  " << key << " = " << value << "\n";
  }

  auto it = params.find("id");
  if (it == params.end()) {
    std::cout << "ERROR: id parameter not found!\n";
    return JsonResponse(req, http::status::bad_request,
                        {{"error", "Missing id parameter"}});
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
  auto it = params.find("id");
  if (it == params.end()) {
    return JsonResponse(req, http::status::bad_request,
                        {{"error", "Missing id parameter"}});
  }

  nlohmann::json response = {
      {"updated", true}, {"id", it->second}, {"message", "Order updated successfully"}};

  return JsonResponse(req, http::status::ok, response);
}

Response OrdersController::Delete(
    const Request& req,
    const std::unordered_map<std::string, std::string>& params) const {
  auto it = params.find("id");
  if (it == params.end()) {
    return JsonResponse(req, http::status::bad_request,
                        {{"error", "Missing id parameter"}});
  }

  nlohmann::json response = {{"deleted", true}, {"id", it->second}};

  return JsonResponse(req, http::status::ok, response);
}
