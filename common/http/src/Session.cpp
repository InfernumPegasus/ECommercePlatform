#include "Session.hpp"

#include <iostream>
#include <thread>

#include "HttpError.hpp"
#include "HttpHelpers.hpp"

namespace {
constexpr int kDefaultHttpVersion = 11;

HttpError MakeParserError(const beast::error_code& ec) {
  if (ec == http::error::header_limit) {
    return HttpError{.status = http::status::request_header_fields_too_large,
                     .code = "header_limit_exceeded",
                     .message = "Request header fields are too large"};
  }

  if (ec == http::error::body_limit) {
    return HttpError{.status = http::status::payload_too_large,
                     .code = "body_limit_exceeded",
                     .message = "Request body is too large"};
  }

  return HttpError{.status = http::status::bad_request,
                   .code = "malformed_http_request",
                   .message = "Malformed HTTP request"};
}

void ConfigureParser(http::request_parser<http::string_body>& parser,
                     const SessionConfig& config, std::size_t handled_requests,
                     beast::tcp_stream& stream) {
  parser.header_limit(config.header_limit_bytes);
  parser.body_limit(config.body_limit_bytes);

  const auto timeout =
      handled_requests == 0 ? config.first_read_timeout : config.keep_alive_timeout;
  stream.expires_after(timeout);
}

}  // namespace

Session::Session(tcp::socket socket, RequestHandler handler, SessionConfig config,
                 OnClose on_close)
    : stream_(std::move(socket)),
      strand_(net::make_strand(stream_.get_executor())),
      handler_(std::move(handler)),
      config_(config),
      on_close_(std::move(on_close)) {}

void Session::Run() {
  net::dispatch(strand_, [self = shared_from_this(), this]() { DoRead(); });
}

void Session::RequestClose() {
  net::dispatch(strand_, [self = shared_from_this(), this]() {
    if (state_ == State::kOpen) {
      state_ = State::kClosing;
    }
  });
}

void Session::ForceClose() {
  net::dispatch(strand_, [self = shared_from_this(), this]() { Close(true); });
}

void Session::DoRead() {
  auto self = shared_from_this();

  parser_.emplace();
  ConfigureParser(*parser_, config_, requests_handled_, stream_);

  http::async_read(
      stream_, buffer_, *parser_,
      net::bind_executor(strand_, [this, self](const beast::error_code& ec, std::size_t) {
        if (ec) {
          HandleReadError(ec);
          return;
        }

        HandleReadSuccess();
      }));
}

void Session::HandleReadError(const beast::error_code& ec) {
  if (ec == beast::error::timeout) {
    std::cout << "[http] client idle timeout, closing connection\n";
    Close(false);
    return;
  }

  if (ec == http::error::end_of_stream) {
    Close(false);
    return;
  }

  Request req;
  req.version(kDefaultHttpVersion);
  auto error = MakeParserError(ec);
  auto res = ErrorResponse(req, error);
  res.keep_alive(false);
  DoWrite(std::move(res));
}

void Session::HandleReadSuccess() {
  auto req = parser_->release();
  ++requests_handled_;
  std::cout << "[http] handling request on thread " << std::this_thread::get_id() << " "
            << http::to_string(req.method()) << " " << req.target() << "\n";
  DoWrite(handler_(req));
}

void Session::DoWrite(Response res) {
  auto self = shared_from_this();

  if (requests_handled_ >= config_.max_requests_per_connection) {
    RequestClose();
  }

  if (state_ == State::kClosing) {
    res.keep_alive(false);
  }

  auto sp = std::make_shared<Response>(std::move(res));

  http::async_write(
      stream_, *sp,
      net::bind_executor(strand_,
                         [this, self, sp](const beast::error_code& ec, std::size_t) {
                           if (ec) {
                             Close(false);
                             return;
                           }

                           const bool close = !sp->keep_alive();

                           buffer_.consume(buffer_.size());

                           if (close) {
                             Close(false);
                           } else {
                             DoRead();
                           }
                         }));
}

void Session::Close(bool force) {
  if (state_ == State::kClosed) {
    return;
  }
  state_ = State::kClosed;

  beast::error_code ep_ec;
  const auto remote = stream_.socket().remote_endpoint(ep_ec);
  if (!ep_ec) {
    std::cout << "[http] connection closed for " << remote.address().to_string() << ":"
              << remote.port() << "\n";
  } else {
    std::cout << "[http] connection closed (endpoint unknown)\n";
  }

  beast::error_code ec;

  if (force) {
    stream_.socket().cancel(ec);
    stream_.socket().shutdown(tcp::socket::shutdown_both, ec);
    stream_.socket().close(ec);
  } else {
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
  }

  if (on_close_) {
    on_close_(shared_from_this());
  }
}
