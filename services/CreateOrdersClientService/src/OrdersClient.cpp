#include "OrdersClient.hpp"

#include <boost/asio/connect.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <string>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

OrdersClient::OrdersClient(std::string host, std::string port)
    : host_(std::move(host)), port_(std::move(port)) {}

HttpResponse OrdersClient::GetOrders(int limit, int offset) {
  return Request(
      "GET",
      "/orders?limit=" + std::to_string(limit) + "&offset=" + std::to_string(offset),
      nullptr);
}

HttpResponse OrdersClient::GetOrderById(int order_id) {
  return Request("GET", "/orders/" + std::to_string(order_id), nullptr);
}

HttpResponse OrdersClient::UpdateOrderName(int order_id, std::string_view new_name) {
  nlohmann::json body = {{"name", std::string(new_name)}};
  return Request("PUT", "/orders/" + std::to_string(order_id) + "/name", &body);
}

HttpResponse OrdersClient::DeleteOrder(int order_id) {
  return Request("DELETE", "/orders/" + std::to_string(order_id), nullptr);
}

HttpResponse OrdersClient::RemoveAll() {
  return Request("POST", "/orders/remove_all", nullptr);
}

HttpResponse OrdersClient::TestMethod(int order_id, std::string_view random_string) {
  return Request(
      "PUT",
      "/orders/" + std::to_string(order_id) + "/name/" + std::string(random_string),
      nullptr);
}

HttpResponse OrdersClient::Request(std::string_view method, std::string_view target,
                                   const nlohmann::json* body) {
  net::io_context ioc;
  tcp::resolver resolver{ioc};
  beast::tcp_stream stream{ioc};

  auto const results = resolver.resolve(host_, port_);
  stream.connect(results);

  http::request<http::string_body> req{http::string_to_verb(std::string(method)),
                                       std::string(target), 11};
  req.set(http::field::host, host_);
  req.set(http::field::user_agent, "CreateOrdersClientService");
  req.set(http::field::accept, "application/json");

  if (body != nullptr) {
    req.set(http::field::content_type, "application/json");
    req.body() = body->dump();
    req.prepare_payload();
  }

  http::write(stream, req);

  beast::flat_buffer buffer;
  http::response<http::string_body> res;
  http::read(stream, buffer, res);

  beast::error_code ec;
  stream.socket().shutdown(tcp::socket::shutdown_both, ec);

  return HttpResponse{static_cast<int>(res.result()), res.body()};
}
