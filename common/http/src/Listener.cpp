#include "Listener.hpp"

#include <boost/asio/strand.hpp>
#include <iostream>

Listener::Listener(boost::asio::io_context& ioc, const tcp::endpoint& endpoint,
                   const Router& router, const HttpServerConfig& config)
    : ioc_(ioc),
      acceptor_(boost::asio::make_strand(ioc)),
      strand_(acceptor_.get_executor()),
      router_(router),
      config_(config) {
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
}

void Listener::Start() { DoAccept(); }

tcp::endpoint Listener::LocalEndpoint() const {
  beast::error_code ec;
  auto endpoint = acceptor_.local_endpoint(ec);
  if (ec) {
    return {};
  }
  return endpoint;
}

void Listener::RequestShutdown() {
  net::dispatch(strand_, [self = shared_from_this()]() {
    if (!self->shutting_down_) {
      self->shutting_down_ = true;

      beast::error_code ec;
      self->acceptor_.close(ec);

      for (const auto& session : self->sessions_) {
        session->RequestClose();
      }
    }
    self->NotifyShutdownCompleteIfDrained();
  });
}

void Listener::ForceShutdown() {
  net::dispatch(strand_, [self = shared_from_this()]() {
    if (!self->shutting_down_) {
      self->shutting_down_ = true;
      beast::error_code ec;
      self->acceptor_.close(ec);
    }

    for (const auto& session : self->sessions_) {
      session->ForceClose();
    }
    self->NotifyShutdownCompleteIfDrained();
  });
}

void Listener::SetOnShutdownComplete(std::function<void()> on_shutdown_complete) {
  net::dispatch(strand_, [self = shared_from_this(),
                          cb = std::move(on_shutdown_complete)]() mutable {
    self->on_shutdown_complete_ = std::move(cb);
    self->NotifyShutdownCompleteIfDrained();
  });
}

void Listener::OnSessionClosed(const std::shared_ptr<Session>& session) {
  net::dispatch(strand_, [self = shared_from_this(), session]() {
    self->sessions_.erase(session);
    self->NotifyShutdownCompleteIfDrained();
  });
}

void Listener::NotifyShutdownCompleteIfDrained() {
  if (!shutting_down_ || shutdown_notified_ || !sessions_.empty()) {
    return;
  }
  shutdown_notified_ = true;
  if (!on_shutdown_complete_) {
    return;
  }

  auto cb = std::move(on_shutdown_complete_);
  net::post(ioc_, std::move(cb));
}

SessionConfig Listener::BuildSessionConfig() const {
  return SessionConfig{
      .first_read_timeout = config_.first_read_timeout,
      .keep_alive_timeout = config_.keep_alive_timeout,
      .header_limit_bytes = config_.header_limit_bytes,
      .body_limit_bytes = config_.body_limit_bytes,
      .max_requests_per_connection = config_.max_requests_per_connection,
  };
}

RequestHandler Listener::BuildHandler() const {
  return [this](const Request& req) { return router_.Route(req); };
}

std::shared_ptr<Session> Listener::CreateSession(tcp::socket socket) {
  auto on_close = [this](std::shared_ptr<Session> session) { OnSessionClosed(session); };
  auto session = std::make_shared<Session>(std::move(socket), BuildHandler(),
                                           BuildSessionConfig(), std::move(on_close));
  sessions_.emplace(session);
  return session;
}

void Listener::HandleAcceptedSocket(tcp::socket socket) {
  beast::error_code ep_ec;
  const auto remote = socket.remote_endpoint(ep_ec);
  if (!ep_ec) {
    std::cout << "[http] client connected from " << remote.address().to_string() << ":"
              << remote.port() << "\n";
  } else {
    std::cout << "[http] client connected (endpoint unknown)\n";
  }

  if (sessions_.size() >= config_.max_connections) {
    std::cout << "[http] max connections reached (" << config_.max_connections
              << "), closing incoming connection\n";
    beast::error_code close_ec;
    socket.shutdown(tcp::socket::shutdown_send, close_ec);
    return;
  }

  auto session = CreateSession(std::move(socket));
  session->Run();
}

void Listener::DoAccept() {
  acceptor_.async_accept(strand_, [self = shared_from_this()](const beast::error_code& ec,
                                                              tcp::socket socket) {
    if (ec) {
      if (self->shutting_down_ || !self->acceptor_.is_open()) {
        return;
      }
    }

    if (!ec) {
      self->HandleAcceptedSocket(std::move(socket));
    }
    if (!self->shutting_down_) {
      self->DoAccept();
    }
  });
}
