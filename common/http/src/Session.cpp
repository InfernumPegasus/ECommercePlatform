#include "Session.hpp"

Session::Session(tcp::socket socket, const Router& router)
    : socket_(std::move(socket)), router_(router) {}

void Session::Run() { DoRead(); }

void Session::DoRead() {
  auto self = shared_from_this();

  http::async_read(socket_, buffer_, req_,
                   [this, self](const boost::beast::error_code& ec, std::size_t) {
                     if (!ec) {
                       DoWrite(router_.Route(req_));
                     }
                   });
}

void Session::DoWrite(Response res) {
  auto self = shared_from_this();

  auto sp = std::make_shared<Response>(std::move(res));

  http::async_write(socket_, *sp,
                    [this, self, sp](boost::beast::error_code ec, std::size_t) {
                      if (ec) return;

                      if (sp->keep_alive()) {
                        req_ = {};
                        buffer_.consume(buffer_.size());
                        DoRead();
                      } else {
                        socket_.shutdown(tcp::socket::shutdown_send, ec);
                      }
                    });
}
