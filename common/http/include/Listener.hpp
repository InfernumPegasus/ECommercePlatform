#pragma once

#include <atomic>
#include <boost/asio/io_context.hpp>

#include "HttpServerConfig.hpp"
#include "Router.hpp"
#include "Session.hpp"

class Listener : public std::enable_shared_from_this<Listener> {
 public:
  Listener(boost::asio::io_context& ioc, const tcp::endpoint& endpoint,
           const Router& router,
           const HttpServerConfig& config = kDefaultHttpServerConfig);
  [[nodiscard]] tcp::endpoint LocalEndpoint() const;

 private:
  void DoAccept();

  boost::asio::io_context& ioc_;
  tcp::acceptor acceptor_;
  const Router& router_;
  const HttpServerConfig& config_;
  std::shared_ptr<std::atomic<std::size_t>> active_connections_;
};
