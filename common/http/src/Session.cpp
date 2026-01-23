#include "Session.hpp"

Session::Session(tcp::socket socket, const Router& router)
    : socket_(std::move(socket)), router_(router) {}

void Session::Run() { DoRead(); }

void Session::DoRead() {
  auto self = shared_from_this();

  http::async_read(socket_, buffer_, req_,
                   [this, self](const beast::error_code& ec, std::size_t) {
                     if (ec == http::error::end_of_stream || ec) {
                       Close();
                       return;
                     }

                     DoWrite(router_.Route(req_));
                   });
}

void Session::DoWrite(Response res) {
  auto self = shared_from_this();

  auto sp = std::make_shared<Response>(std::move(res));

  http::async_write(socket_, *sp,
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
  beast::error_code ec;

  socket_.shutdown(tcp::socket::shutdown_send, ec);
}
