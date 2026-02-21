#include <chrono>
#include <iostream>
#include <string>
#include <thread>

#include <nlohmann/json.hpp>

#include "OrdersClient.hpp"

namespace {
void PrintResponse(std::string_view label, const HttpResponse& res) {
  std::cout << "\n== " << label << " ==\n";
  std::cout << "status: " << res.status << "\n";

  if (res.body.empty()) {
    std::cout << "body: <empty>\n";
    return;
  }

  auto json = nlohmann::json::parse(res.body, nullptr, false);
  if (json.is_discarded()) {
    std::cout << "body: " << res.body << "\n";
    return;
  }

  std::cout << "body: " << json.dump(2) << "\n";
}
}  // namespace

int main() {
  try {
    OrdersClient client("127.0.0.1", "1234");

    PrintResponse("GET /orders", client.GetOrders(5, 0));
    PrintResponse("GET /orders/1", client.GetOrderById(1));
    PrintResponse("PUT /orders/1/name", client.UpdateOrderName(1, "Order #1"));
    PrintResponse("PUT /orders/1/name/random", client.TestMethod(1, "random"));
    PrintResponse("DELETE /orders/1", client.DeleteOrder(1));
    PrintResponse("POST /orders/remove_all", client.RemoveAll());

    std::cout << "\nClient finished requests.\n";
  } catch (const std::exception& e) {
    std::cerr << "Client error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
