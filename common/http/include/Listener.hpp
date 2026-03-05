#pragma once

#include <boost/asio/io_context.hpp>
#include <functional>
#include <unordered_set>

#include "HttpNet.hpp"
#include "HttpServerConfig.hpp"
#include "Session.hpp"

class Router;

class Listener : public std::enable_shared_from_this<Listener> {
 public:
  using ListenerHandler = std::function<Response(const Request&)>;

  Listener(boost::asio::io_context& ioc, const tcp::endpoint& endpoint,
           ListenerHandler handler,
           const HttpServerConfig& config = kDefaultHttpServerConfig);
  Listener(boost::asio::io_context& ioc, const tcp::endpoint& endpoint,
           std::shared_ptr<const Router> router,
           const HttpServerConfig& config = kDefaultHttpServerConfig);
  void Start();
  [[nodiscard]] tcp::endpoint LocalEndpoint() const;
  void RequestShutdown();
  void ForceShutdown();
  void SetOnShutdownComplete(std::function<void()> on_shutdown_complete);

 private:
  void HandleAcceptedSocket(tcp::socket socket);
  std::shared_ptr<Session> CreateSession(tcp::socket socket);
  void DoAccept();
  void OnSessionClosed(const std::shared_ptr<Session>& session);
  void NotifyShutdownCompleteIfDrained();

  net::any_io_executor callback_executor_;
  tcp::acceptor acceptor_;
  net::strand<net::any_io_executor> strand_;
  ListenerHandler handler_;
  SessionConfig session_config_;
  std::size_t max_connections_;
  std::unordered_set<std::shared_ptr<Session>> sessions_;
  std::function<void()> on_shutdown_complete_;
  bool shutting_down_ = false;
  bool shutdown_notified_ = false;
};
