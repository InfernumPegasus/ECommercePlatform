#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/url.hpp>
#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>
#include <pqxx/pqxx>
#include <string>
#include <thread>
#include <vector>

#include "CreateOrdersService.hpp"
#include "DbMapping.hpp"
#include "HttpServerConfig.hpp"
#include "Listener.hpp"
#include "User.hpp"

template <>
struct DbMapping<User> {
  static constexpr auto fields =
      std::make_tuple(std::pair{"id", &User::id}, std::pair{"status", &User::status},
                      std::pair{"created_at", &User::created_at},
                      std::pair{"updated_at", &User::updated_at});
};

void GetAllUsers(pqxx::work& work) {
  static constexpr auto query = "SELECT * FROM \"Users\";";

  const auto rows = work.exec(query).expect_columns(4);

  std::println(std::cout, "printing all users:");

  for (auto r : rows) {
    const auto user = MapRowTo<User>(r);

    std::println(std::cout, "user {}: {}, created_at {}, updated_at {}", user.id,
                 user.status, user.created_at, user.updated_at);
  }
}

void GetAllUsers(pqxx::work& work, nlohmann::json& json) {
  static constexpr auto query = "SELECT * FROM \"Users\";";

  for (const auto rows = work.exec(query).expect_columns(4); auto r : rows) {
    const auto user = MapRowTo<User>(r);
    json["users"].push_back(user);
  }
}

std::string FormConnectionString(std::string_view dbname, std::string_view user,
                                 std::string_view password, std::string_view host,
                                 int port) {
  return std::format("dbname={} user={} password={} host={} port={}", dbname, user,
                     password, host, port);
}

int main() {
  try {
    const auto connectionString =
        FormConnectionString("ecommerce", "ecommerce", "ecommerce", "localhost", 5432);
    // pqxx::connection conn(connectionString);
    // pqxx::work work{conn};

    boost::asio::io_context ioc;

    auto router = std::make_shared<Router>();

    auto orders = std::make_shared<OrdersController>();
    orders->RegisterRoutes(*router);
    orders->PrintAvailableRoutes();

    tcp::endpoint endpoint{tcp::v4(), 1234};
    constexpr HttpServerConfig kServerConfig{
        .io_threads = 4,
        .first_read_timeout = std::chrono::seconds(30),
        .keep_alive_timeout = std::chrono::seconds(15),
        .header_limit_bytes = 8 * 1024,
        .body_limit_bytes = 1024 * 1024,
        .max_requests_per_connection = 50,
        .max_connections = 1000,
    };

    auto listener = std::make_shared<Listener>(ioc, endpoint, router, kServerConfig);
    listener->Start();
    net::steady_timer shutdownTimer{ioc};
    bool shutdownStarted = false;
    constexpr auto kGracefulShutdownTimeout = std::chrono::seconds(10);

    listener->SetOnShutdownComplete([&shutdownTimer, &ioc]() {
      shutdownTimer.cancel();
      std::cout << "[http] graceful shutdown complete, stopping io_context\n";
      ioc.stop();
    });

    auto beginShutdown = [listener, &shutdownTimer, &shutdownStarted, &ioc,
                          kGracefulShutdownTimeout]() {
      if (shutdownStarted) {
        return;
      }
      shutdownStarted = true;

      std::cout << "[http] shutdown requested, draining active sessions\n";
      listener->RequestShutdown();

      shutdownTimer.expires_after(kGracefulShutdownTimeout);
      shutdownTimer.async_wait([listener, &ioc](const boost::system::error_code& ec) {
        if (ec == net::error::operation_aborted) {
          return;
        }
        if (ec) {
          std::cout << "[http] shutdown timer error: " << ec.message() << "\n";
          return;
        }

        std::cout << "[http] graceful shutdown timeout reached, forcing close\n";
        listener->ForceShutdown();
        ioc.stop();
      });
    };

    net::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait([beginShutdown](const boost::system::error_code& ec, int signal) {
      if (ec) {
        return;
      }
      std::cout << "[http] received signal " << signal << "\n";
      beginShutdown();
    });

    std::println(std::cout, "Listening on http://{}:{}\n", endpoint.address().to_string(),
                 endpoint.port());
    std::vector<std::thread> threads;
    threads.reserve(kServerConfig.io_threads);
    for (std::size_t i = 0; i < kServerConfig.io_threads; ++i) {
      threads.emplace_back([&ioc]() { ioc.run(); });
    }

    for (auto& t : threads) {
      t.join();
    }
  } catch (const std::exception& e) {
    std::cerr << "Fatal error: " << e.what() << "\n";
  }
}
