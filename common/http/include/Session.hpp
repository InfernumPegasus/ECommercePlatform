#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <memory>

#include "Router.hpp"

namespace net = boost::asio;
using tcp = net::ip::tcp;

class Session : public std::enable_shared_from_this<Session> {
 public:
  Session(tcp::socket socket, const Router& router);

  void run();

 private:
  void do_read();
  
  void do_write(Response res);

  tcp::socket socket_;
  boost::beast::flat_buffer buffer_;
  Request req_;
  const Router& router_;
};
