#include "Session.hpp"

#include <iostream>

Session::Session(tcp::socket socket, const Router& router, const HttpServerConfig& config,
                 const std::shared_ptr<std::atomic<std::size_t>>& active_connections)
    : stream_(std::move(socket)),
      router_(router),
      config_(config),
      active_connections_(active_connections) {}

void Session::Run() { DoRead(); }

void Session::DoRead() {
  auto self = shared_from_this();

  parser_.emplace();
  parser_->header_limit(config_.header_limit_bytes);
  parser_->body_limit(config_.body_limit_bytes);

  const auto timeout =
      requests_handled_ == 0 ? config_.first_read_timeout : config_.keep_alive_timeout;
  stream_.expires_after(timeout);

  http::async_read(stream_, buffer_, *parser_,
                   [this, self](const beast::error_code& ec, std::size_t) {
                     if (ec == beast::error::timeout) {
                       std::cout << "[http] client idle timeout, closing connection\n";
                       Close();
                       return;
                     }
                     if (ec == http::error::end_of_stream || ec) {
                       Close();
                       return;
                     }

                     req_ = parser_->release();
                     ++requests_handled_;
                     DoWrite(router_.Route(req_));
                   });
}

void Session::DoWrite(Response res) {
  auto self = shared_from_this();

  if (requests_handled_ >= config_.max_requests_per_connection) {
    res.keep_alive(false);
  }

  auto sp = std::make_shared<Response>(std::move(res));

  http::async_write(stream_, *sp,
                    [this, self, sp](const beast::error_code& ec, std::size_t) {
                      if (ec) {
                        Close();
                        return;
                      }

                      const bool close = !sp->keep_alive();

                      buffer_.consume(buffer_.size());
                      req_ = {};

                      if (close) {
                        Close();
                      } else {
                        DoRead();
                      }
                    });
}

void Session::Close() {
  if (closed_) {
    return;
  }
  closed_ = true;

  boost::beast::error_code ep_ec;
  const auto remote = stream_.socket().remote_endpoint(ep_ec);
  if (!ep_ec) {
    std::cout << "[http] connection closed for " << remote.address().to_string()
              << ":" << remote.port() << "\n";
  } else {
    std::cout << "[http] connection closed (endpoint unknown)\n";
  }

  beast::error_code ec;

  stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

  if (active_connections_) {
    active_connections_->fetch_sub(1, std::memory_order_relaxed);
  }
}
