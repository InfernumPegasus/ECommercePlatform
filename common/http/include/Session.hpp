#pragma once

#include <atomic>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <memory>
#include <optional>

#include "HttpServerConfig.hpp"
#include "Router.hpp"

namespace net = boost::asio;
namespace beast = boost::beast;
using tcp = net::ip::tcp;
namespace http = beast::http;

class Session : public std::enable_shared_from_this<Session> {
 public:
  Session(tcp::socket socket, const Router& router, const HttpServerConfig& config,
          const std::shared_ptr<std::atomic<std::size_t>>& active_connections);

  void Run();

 private:
  void DoRead();
  void DoWrite(Response res);
  void Close();

  beast::tcp_stream stream_;
  net::strand<net::any_io_executor> strand_;
  beast::flat_buffer buffer_;
  Request req_;
  std::optional<http::request_parser<http::string_body>> parser_;
  const Router& router_;
  const HttpServerConfig& config_;
  std::shared_ptr<std::atomic<std::size_t>> active_connections_;
  std::size_t requests_handled_ = 0;
  bool closed_ = false;
};
