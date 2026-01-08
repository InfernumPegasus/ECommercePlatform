#include "Listener.hpp"

Listener::Listener(boost::asio::io_context& ioc, const tcp::endpoint& endpoint,
                   const Router& router)
    : ioc_(ioc), acceptor_(boost::asio::make_strand(ioc)), router_(router) {
  boost::beast::error_code ec;

  acceptor_.open(endpoint.protocol(), ec);
  if (ec) {
    throw std::runtime_error("Failed to open acceptor: " + ec.message());
  }

  acceptor_.set_option(boost::asio::socket_base::reuse_address(true), ec);
  if (ec) {
    acceptor_.close();
    throw std::runtime_error("Failed to set reuse_address option: " + ec.message());
  }

  acceptor_.bind(endpoint, ec);
  if (ec) {
    acceptor_.close();
    throw std::runtime_error("Failed to bind to endpoint " +
                             endpoint.address().to_string() + ":" +
                             std::to_string(endpoint.port()) + ": " + ec.message());
  }

  acceptor_.listen(boost::asio::socket_base::max_listen_connections, ec);
  if (ec) {
    acceptor_.close();
    throw std::runtime_error("Failed to start listening: " + ec.message());
  }

  DoAccept();
}

void Listener::DoAccept() {
  acceptor_.async_accept(boost::asio::make_strand(ioc_),
                         [this](const boost::beast::error_code& ec, tcp::socket socket) {
                           if (!ec) {
                             std::make_shared<Session>(std::move(socket), router_)->Run();
                           }
                           DoAccept();
                         });
}
