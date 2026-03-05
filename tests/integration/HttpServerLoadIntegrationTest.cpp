#include <gtest/gtest.h>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <chrono>
#include <cstddef>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include "HttpHelpers.hpp"
#include "HttpServerConfig.hpp"
#include "Listener.hpp"
#include "Router.hpp"

namespace net = boost::asio;
namespace http = boost::beast::http;
namespace beast = boost::beast;
using tcp = net::ip::tcp;

namespace {

class LoadController {
 public:
  Response Ping(const RequestContext& ctx) {
    return JsonResponse(ctx.GetRequest(), http::status::ok, {{"ok", true}});
  }
};

class TestServer {
 public:
  explicit TestServer(const HttpServerConfig& config) : ioc_(config.io_threads) {
    controller_ = std::make_shared<LoadController>();
    router_ = std::make_shared<Router>();
    router_->AddRoute(http::verb::get, "/ping", &LoadController::Ping, controller_);

    try {
      listener_ =
          std::make_shared<Listener>(ioc_, tcp::endpoint{tcp::v4(), 0}, router_, config);
      listener_->Start();
      port_ = listener_->LocalEndpoint().port();
      for (std::size_t i = 0; i < config.io_threads; ++i) {
        threads_.emplace_back([this]() { ioc_.run(); });
      }
      available_ = true;
    } catch (const std::exception& ex) {
      init_error_ = ex.what();
      available_ = false;
    }
  }

  ~TestServer() {
    ioc_.stop();
    for (auto& thread : threads_) {
      if (thread.joinable()) {
        thread.join();
      }
    }
  }

  [[nodiscard]] bool IsAvailable() const { return available_; }
  [[nodiscard]] unsigned short Port() const { return port_; }
  [[nodiscard]] const std::string& InitError() const { return init_error_; }

 private:
  net::io_context ioc_;
  std::shared_ptr<Router> router_;
  std::shared_ptr<LoadController> controller_;
  std::shared_ptr<Listener> listener_;
  std::vector<std::thread> threads_;
  unsigned short port_ = 0;
  bool available_ = false;
  std::string init_error_;
};

struct LoadStats {
  std::size_t success = 0;
  std::size_t failures = 0;
  std::chrono::microseconds total_latency{0};
};

LoadStats RunClientBatch(unsigned short port, std::size_t requests_per_client) {
  LoadStats stats{};
  net::io_context ioc;
  beast::tcp_stream stream{ioc};

  beast::error_code ec;
  stream.connect(tcp::endpoint{net::ip::make_address("127.0.0.1"), port}, ec);
  if (ec) {
    stats.failures = requests_per_client;
    return stats;
  }

  beast::flat_buffer buffer;
  for (std::size_t i = 0; i < requests_per_client; ++i) {
    http::request<http::string_body> req{http::verb::get, "/ping", 11};
    req.set(http::field::host, "127.0.0.1");
    req.keep_alive(true);

    auto start = std::chrono::steady_clock::now();
    http::write(stream, req, ec);
    if (ec) {
      stats.failures += (requests_per_client - i);
      break;
    }

    http::response<http::string_body> res;
    http::read(stream, buffer, res, ec);
    if (ec) {
      stats.failures += (requests_per_client - i);
      break;
    }

    auto elapsed = std::chrono::steady_clock::now() - start;
    stats.total_latency += std::chrono::duration_cast<std::chrono::microseconds>(elapsed);

    if (res.result() == http::status::ok) {
      ++stats.success;
    } else {
      ++stats.failures;
    }
    buffer.consume(buffer.size());
  }

  stream.socket().shutdown(tcp::socket::shutdown_both, ec);
  return stats;
}

}  // namespace

TEST(HttpServerLoadIntegrationTest, ParallelClientsOverTcp) {
  HttpServerConfig config = kDefaultHttpServerConfig;
  config.io_threads = 4;
  config.max_connections = 256;
  config.max_requests_per_connection = 500;
  config.keep_alive_timeout = std::chrono::seconds(3);
  config.first_read_timeout = std::chrono::seconds(3);

  TestServer server(config);
  if (!server.IsAvailable()) {
    GTEST_SKIP() << "Server is unavailable in this environment: " << server.InitError();
  }

  constexpr std::size_t kClients = 24;
  constexpr std::size_t kRequestsPerClient = 120;

  std::vector<std::thread> clients;
  clients.reserve(kClients);
  std::vector<LoadStats> per_client(kClients);

  auto start = std::chrono::steady_clock::now();
  for (std::size_t i = 0; i < kClients; ++i) {
    clients.emplace_back([&per_client, i, &server]() {
      per_client[i] = RunClientBatch(server.Port(), kRequestsPerClient);
    });
  }

  for (auto& client : clients) {
    client.join();
  }
  auto total_elapsed = std::chrono::steady_clock::now() - start;

  std::size_t success = 0;
  std::size_t failures = 0;
  std::chrono::microseconds total_latency{0};
  for (const auto& s : per_client) {
    success += s.success;
    failures += s.failures;
    total_latency += s.total_latency;
  }

  const auto total_requests = kClients * kRequestsPerClient;
  const auto elapsed_ms =
      std::chrono::duration_cast<std::chrono::milliseconds>(total_elapsed).count();
  const auto avg_latency_us = success == 0 ? 0 : (total_latency.count() / success);

  RecordProperty("clients", static_cast<int>(kClients));
  RecordProperty("requests_per_client", static_cast<int>(kRequestsPerClient));
  RecordProperty("total_requests", static_cast<int>(total_requests));
  RecordProperty("success_requests", static_cast<int>(success));
  RecordProperty("failed_requests", static_cast<int>(failures));
  RecordProperty("elapsed_ms", static_cast<int>(elapsed_ms));
  RecordProperty("avg_latency_us", static_cast<int>(avg_latency_us));

  EXPECT_EQ(success + failures, total_requests);
  EXPECT_EQ(failures, 0U);
}
