#pragma once

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <functional>
#include <memory>
#include <optional>

#include "HttpNet.hpp"
#include "HttpServerConfig.hpp"
#include "HttpTypes.hpp"

class Session;

using RequestHandler = std::move_only_function<Response(const Request&)>;
using OnClose = std::move_only_function<void(std::shared_ptr<Session>)>;

class Session : public std::enable_shared_from_this<Session> {
 public:
  Session(tcp::socket socket, RequestHandler handler, SessionConfig config,
          OnClose on_close);

  void Run();
  void RequestClose();
  void ForceClose();

 private:
  enum class State : uint8_t { kOpen, kClosing, kClosed };

 private:
  void DoRead();
  void DoWrite(Response res);
  void Close(bool force);
  void HandleReadError(const beast::error_code& ec);
  void HandleReadSuccess();

  beast::tcp_stream stream_;
  net::strand<net::any_io_executor> strand_;
  beast::flat_buffer buffer_;
  std::optional<http::request_parser<http::string_body>> parser_;
  RequestHandler handler_;
  SessionConfig config_;
  OnClose on_close_;
  std::size_t requests_handled_ = 0;
  State state_{State::kOpen};
};
