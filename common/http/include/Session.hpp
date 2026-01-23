#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <memory>

#include "Router.hpp"

namespace net = boost::asio;
namespace beast = boost::beast;
using tcp = net::ip::tcp;
namespace http = beast::http;

class Session : public std::enable_shared_from_this<Session> {
 public:
  Session(tcp::socket socket, const Router& router);

  void Run();

 private:
  void DoRead();
  void DoWrite(Response res);
  void Close();

  tcp::socket socket_;
  beast::flat_buffer buffer_;
  Request req_;
  const Router& router_;
};
