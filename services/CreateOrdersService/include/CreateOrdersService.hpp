#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

// Produces an HTTP response for the given request
http::response<http::string_body> handle_request(
    http::request<http::string_body> const& req);

// Handles a single HTTP connection
class Session : public std::enable_shared_from_this<Session> {
 public:
  explicit Session(tcp::socket socket);
  void run();

 private:
  void do_read();
  void do_write(http::response<http::string_body> res);

  tcp::socket socket_;
  beast::flat_buffer buffer_;
  http::request<http::string_body> req_;
};

// Accepts incoming connections and launches sessions
class Listener : public std::enable_shared_from_this<Listener> {
 public:
  Listener(net::io_context& ioc, tcp::endpoint endpoint);

 private:
  void do_accept();

  net::io_context& ioc_;
  tcp::acceptor acceptor_;
};
