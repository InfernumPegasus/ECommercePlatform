#include "Session.hpp"

Session::Session(tcp::socket socket, const Router& router)
    : socket_(std::move(socket)), router_(router) {}

void Session::run() { do_read(); }

void Session::do_read() {
  auto self = shared_from_this();
  http::async_read(socket_, buffer_, req_,
                   [this, self](boost::beast::error_code ec, std::size_t) {
                     if (!ec) {
                       do_write(router_.route(req_));
                     }
                   });
}

void Session::do_write(Response res) {
  auto self = shared_from_this();
  auto sp = std::make_shared<Response>(std::move(res));

  http::async_write(socket_, *sp,
                    [this, self, sp](boost::beast::error_code ec, std::size_t) {
                      socket_.shutdown(tcp::socket::shutdown_send, ec);
                    });
}
