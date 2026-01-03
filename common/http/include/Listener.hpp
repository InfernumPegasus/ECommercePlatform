#pragma once

#include <boost/asio/io_context.hpp>
#include <boost/asio/strand.hpp>

#include "Router.hpp"
#include "Session.hpp"

class Listener : public std::enable_shared_from_this<Listener> {
 public:
  Listener(boost::asio::io_context& ioc, const tcp::endpoint& endpoint, const Router& router);

 private:
  void DoAccept();

  boost::asio::io_context& ioc_;
  tcp::acceptor acceptor_;
  const Router& router_;
};
