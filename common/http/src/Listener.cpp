#include "Listener.hpp"

#include <boost/asio/strand.hpp>
#include <iostream>

Listener::Listener(boost::asio::io_context& ioc, const tcp::endpoint& endpoint,
                   const Router& router, const HttpServerConfig& config)
    : ioc_(ioc),
      acceptor_(boost::asio::make_strand(ioc)),
      router_(router),
      config_(config),
      active_connections_(std::make_shared<std::atomic<std::size_t>>(0)) {
  beast::error_code ec;

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

tcp::endpoint Listener::LocalEndpoint() const {
  beast::error_code ec;
  auto endpoint = acceptor_.local_endpoint(ec);
  if (ec) {
    return {};
  }
  return endpoint;
}

void Listener::DoAccept() {
  acceptor_.async_accept(
      boost::asio::make_strand(ioc_),
      [this](const beast::error_code& ec, tcp::socket socket) {
        if (!ec) {
          beast::error_code ep_ec;
          const auto remote = socket.remote_endpoint(ep_ec);
          if (!ep_ec) {
            std::cout << "[http] client connected from " << remote.address().to_string()
                      << ":" << remote.port() << "\n";
          } else {
            std::cout << "[http] client connected (endpoint unknown)\n";
          }

          const auto current = active_connections_->load(std::memory_order_relaxed);
          if (current >= config_.max_connections) {
            std::cout << "[http] max connections reached (" << config_.max_connections
                      << "), closing incoming connection\n";
            beast::error_code close_ec;
            socket.shutdown(tcp::socket::shutdown_send, close_ec);
          } else {
            active_connections_->fetch_add(1, std::memory_order_relaxed);
            std::make_shared<Session>(std::move(socket), router_, config_,
                                      active_connections_)
                ->Run();
          }
        }
        DoAccept();
      });
}
