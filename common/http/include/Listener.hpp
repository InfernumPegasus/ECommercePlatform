#pragma once

#include <boost/asio/io_context.hpp>
#include <functional>
#include <unordered_set>

#include "HttpNet.hpp"
#include "HttpServerConfig.hpp"
#include "Router.hpp"
#include "Session.hpp"

class Listener : public std::enable_shared_from_this<Listener> {
 public:
  Listener(boost::asio::io_context& ioc, const tcp::endpoint& endpoint,
           const Router& router,
           const HttpServerConfig& config = kDefaultHttpServerConfig);
  void Start();
  [[nodiscard]] tcp::endpoint LocalEndpoint() const;
  void RequestShutdown();
  void ForceShutdown();
  void SetOnShutdownComplete(std::function<void()> on_shutdown_complete);

 private:
  SessionConfig BuildSessionConfig() const;
  RequestHandler BuildHandler() const;
  void HandleAcceptedSocket(tcp::socket socket);
  std::shared_ptr<Session> CreateSession(tcp::socket socket);
  void DoAccept();
  void OnSessionClosed(const std::shared_ptr<Session>& session);
  void NotifyShutdownCompleteIfDrained();

  boost::asio::io_context& ioc_;
  tcp::acceptor acceptor_;
  net::strand<net::any_io_executor> strand_;
  const Router& router_;
  const HttpServerConfig& config_;
  std::unordered_set<std::shared_ptr<Session>> sessions_;
  std::function<void()> on_shutdown_complete_;
  bool shutting_down_ = false;
  bool shutdown_notified_ = false;
};
