#include <gtest/gtest.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/http.hpp>
#include <chrono>
#include <string>
#include <thread>

#include "HttpHelpers.hpp"
#include "HttpServerConfig.hpp"
#include "Listener.hpp"
#include "Router.hpp"

namespace net = boost::asio;
namespace http = beast::http;
using tcp = net::ip::tcp;
using namespace std::chrono_literals;

namespace {

class TestController {
 public:
  Response Ping(const RequestContext& ctx) {
    return JsonResponse(ctx.GetRequest(), http::status::ok, {{"ok", true}});
  }
};

class TestServer {
 public:
  explicit TestServer(const HttpServerConfig& config) : ioc_(1) {
    router_.AddRoute(http::verb::get, "/ping", &TestController::Ping, &controller_);
    listener_ =
        std::make_shared<Listener>(ioc_, tcp::endpoint{tcp::v4(), 0}, router_, config);
    port_ = listener_->LocalEndpoint().port();
    thread_ = std::thread([this]() { ioc_.run(); });
  }

  ~TestServer() {
    ioc_.stop();
    if (thread_.joinable()) {
      thread_.join();
    }
  }

  unsigned short Port() const { return port_; }

 private:
  net::io_context ioc_;
  Router router_;
  TestController controller_;
  std::shared_ptr<Listener> listener_;
  unsigned short port_ = 0;
  std::thread thread_;
};

bool WaitForSocketClose(tcp::socket& socket, std::chrono::milliseconds timeout) {
  socket.non_blocking(true);
  auto start = std::chrono::steady_clock::now();
  char data[1];

  while (std::chrono::steady_clock::now() - start < timeout) {
    boost::system::error_code ec;
    socket.read_some(net::buffer(data), ec);

    if (ec == net::error::eof) {
      return true;
    }

    if (ec == net::error::would_block || ec == net::error::try_again) {
      std::this_thread::sleep_for(50ms);
      continue;
    }

    if (ec) {
      return true;
    }
  }

  return false;
}

tcp::socket ConnectSocket(net::io_context& ioc, unsigned short port) {
  tcp::socket socket{ioc};
  socket.connect(tcp::endpoint{net::ip::make_address("127.0.0.1"), port});
  return socket;
}

}  // namespace

TEST(HttpServerBehaviorIntegrationTest, IdleTimeoutClosesConnection) {
  HttpServerConfig config = kDefaultHttpServerConfig;
  config.first_read_timeout = 1s;
  config.keep_alive_timeout = 1s;

  TestServer server(config);
  net::io_context ioc;
  auto socket = ConnectSocket(ioc, server.Port());

  EXPECT_TRUE(WaitForSocketClose(socket, 3s));
}

TEST(HttpServerBehaviorIntegrationTest, KeepAliveTimeoutClosesConnection) {
  HttpServerConfig config = kDefaultHttpServerConfig;
  config.first_read_timeout = 2s;
  config.keep_alive_timeout = 1s;

  TestServer server(config);

  net::io_context ioc;
  beast::tcp_stream stream{ioc};
  stream.connect(tcp::endpoint{net::ip::make_address("127.0.0.1"), server.Port()});

  http::request<http::string_body> req{http::verb::get, "/ping", 11};
  req.set(http::field::host, "127.0.0.1");
  req.keep_alive(true);

  http::write(stream, req);
  beast::flat_buffer buffer;
  http::response<http::string_body> res;
  http::read(stream, buffer, res);

  EXPECT_TRUE(WaitForSocketClose(stream.socket(), 3s));
}

TEST(HttpServerBehaviorIntegrationTest, MaxRequestsPerConnectionClosesAfterLimit) {
  HttpServerConfig config = kDefaultHttpServerConfig;
  config.max_requests_per_connection = 2;
  config.keep_alive_timeout = 3s;

  TestServer server(config);

  net::io_context ioc;
  beast::tcp_stream stream{ioc};
  stream.connect(tcp::endpoint{net::ip::make_address("127.0.0.1"), server.Port()});

  http::request<http::string_body> req{http::verb::get, "/ping", 11};
  req.set(http::field::host, "127.0.0.1");
  req.keep_alive(true);

  beast::flat_buffer buffer;
  http::response<http::string_body> res1;
  http::write(stream, req);
  http::read(stream, buffer, res1);
  EXPECT_TRUE(res1.keep_alive());

  buffer.consume(buffer.size());
  http::response<http::string_body> res2;
  http::write(stream, req);
  http::read(stream, buffer, res2);
  EXPECT_FALSE(res2.keep_alive());

  EXPECT_TRUE(WaitForSocketClose(stream.socket(), 3s));
}

TEST(HttpServerBehaviorIntegrationTest, HeaderLimitIsEnforced) {
  HttpServerConfig config = kDefaultHttpServerConfig;
  config.header_limit_bytes = 64;
  config.first_read_timeout = 2s;

  TestServer server(config);

  net::io_context ioc;
  auto socket = ConnectSocket(ioc, server.Port());
  std::string big_header(200, 'A');
  std::string request =
      "GET /ping HTTP/1.1\r\nHost: 127.0.0.1\r\nX-Fill: " + big_header + "\r\n\r\n";
  boost::system::error_code ec;
  socket.write_some(net::buffer(request), ec);
  ASSERT_FALSE(ec);

  EXPECT_TRUE(WaitForSocketClose(socket, 3s));
}

TEST(HttpServerBehaviorIntegrationTest, BodyLimitIsEnforced) {
  HttpServerConfig config = kDefaultHttpServerConfig;
  config.body_limit_bytes = 32;
  config.first_read_timeout = 2s;

  TestServer server(config);

  net::io_context ioc;
  beast::tcp_stream stream{ioc};
  stream.connect(tcp::endpoint{net::ip::make_address("127.0.0.1"), server.Port()});

  std::string body(64, 'B');
  http::request<http::string_body> req{http::verb::put, "/ping", 11};
  req.set(http::field::host, "127.0.0.1");
  req.set(http::field::content_type, "text/plain");
  req.body() = body;
  req.prepare_payload();

  http::write(stream, req);

  EXPECT_TRUE(WaitForSocketClose(stream.socket(), 3s));
}

TEST(HttpServerBehaviorIntegrationTest, MaxConnectionsIsEnforced) {
  HttpServerConfig config = kDefaultHttpServerConfig;
  config.max_connections = 1;
  config.first_read_timeout = 2s;

  TestServer server(config);

  net::io_context ioc;
  auto first = ConnectSocket(ioc, server.Port());
  auto second = ConnectSocket(ioc, server.Port());

  EXPECT_TRUE(WaitForSocketClose(second, 3s));

  boost::system::error_code ec;
  first.shutdown(tcp::socket::shutdown_both, ec);
  first.close(ec);
}
