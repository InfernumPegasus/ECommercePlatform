#include "Listener.hpp"

#include <iostream>

Listener::Listener(boost::asio::io_context& ioc, tcp::endpoint endpoint,
                   const Router& router)
    : ioc_(ioc), acceptor_(boost::asio::make_strand(ioc)), router_(router) {
  boost::beast::error_code ec;

  acceptor_.open(endpoint.protocol(), ec);
  acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
  acceptor_.bind(endpoint, ec);
  acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);

  do_accept();
}

void Listener::do_accept() {
  acceptor_.async_accept(boost::asio::make_strand(ioc_),
                         [this](boost::beast::error_code ec, tcp::socket socket) {
                           if (!ec) {
                             std::make_shared<Session>(std::move(socket), router_)->run();
                           }
                           do_accept();
                         });
}
