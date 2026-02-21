#pragma once

#include <string>
#include <string_view>

#include <nlohmann/json.hpp>

struct HttpResponse {
  int status = 0;
  std::string body;
};

class OrdersClient {
 public:
  OrdersClient(std::string host, std::string port);

  HttpResponse GetOrders(int limit, int offset);
  HttpResponse GetOrderById(int order_id);
  HttpResponse UpdateOrderName(int order_id, std::string_view new_name);
  HttpResponse DeleteOrder(int order_id);
  HttpResponse RemoveAll();
  HttpResponse TestMethod(int order_id, std::string_view random_string);

 private:
  HttpResponse Request(std::string_view method, std::string_view target,
                       const nlohmann::json* body);

  std::string host_;
  std::string port_;
};
