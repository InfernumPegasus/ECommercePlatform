#include <gtest/gtest.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "IController.hpp"
#include "RouteDSL.hpp"
#include "Router.hpp"

namespace http = boost::beast::http;

namespace {

class LoadController : public IController<LoadController> {
 public:
  static constexpr auto Routes() {
    return route_dsl::Routes(
        route_dsl::GET("/health", &LoadController::Health),
        route_dsl::GET("/orders/{id:int}", &LoadController::GetOrder));
  }

  Response Health(const RequestContext& ctx) {
    Response res{http::status::ok, ctx.GetRequest().version()};
    res.keep_alive(ctx.GetRequest().keep_alive());
    res.body() = "ok";
    res.prepare_payload();
    return res;
  }

  Response GetOrder(const RequestContext& ctx) {
    const auto id = ctx.GetPathParameters().Required<int>("id");
    Response res{http::status::ok, ctx.GetRequest().version()};
    res.keep_alive(ctx.GetRequest().keep_alive());
    res.body() = std::to_string(id);
    res.prepare_payload();
    return res;
  }
};

Request MakeRequest(http::verb method, std::string_view target) {
  Request req;
  req.method(method);
  req.target(target);
  req.version(11);
  req.set(http::field::host, "localhost");
  req.keep_alive(true);
  return req;
}

}  // namespace

TEST(HttpRoutingLoadIntegrationTest, HighVolumeSequentialRouting) {
  auto controller = std::make_shared<LoadController>();
  Router router;
  controller->RegisterRoutes(router);

  constexpr std::size_t kIterations = 50000;
  auto start = std::chrono::steady_clock::now();

  for (std::size_t i = 0; i < kIterations; ++i) {
    auto response = router.Route(MakeRequest(http::verb::get, "/health"));
    ASSERT_EQ(response.result(), http::status::ok);

    response = router.Route(MakeRequest(http::verb::get, "/orders/123"));
    ASSERT_EQ(response.result(), http::status::ok);
    ASSERT_EQ(response.body(), "123");
  }

  auto elapsed = std::chrono::steady_clock::now() - start;
  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
  RecordProperty("sequential_requests", static_cast<int>(kIterations * 2));
  RecordProperty("sequential_elapsed_ms", static_cast<int>(ms));
}

TEST(HttpRoutingLoadIntegrationTest, ParallelRoutingStability) {
  auto controller = std::make_shared<LoadController>();
  Router router;
  controller->RegisterRoutes(router);

  constexpr std::size_t kThreads = 8;
  constexpr std::size_t kIterationsPerThread = 5000;
  std::atomic<std::size_t> failures{0};

  std::vector<std::thread> threads;
  threads.reserve(kThreads);

  auto start = std::chrono::steady_clock::now();
  for (std::size_t t = 0; t < kThreads; ++t) {
    threads.emplace_back([&router, &failures]() {
      for (std::size_t i = 0; i < kIterationsPerThread; ++i) {
        auto res = router.Route(MakeRequest(http::verb::get, "/orders/42"));
        if (res.result() != http::status::ok || res.body() != "42") {
          failures.fetch_add(1, std::memory_order_relaxed);
        }
      }
    });
  }

  for (auto& thread : threads) {
    thread.join();
  }

  auto elapsed = std::chrono::steady_clock::now() - start;
  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
  RecordProperty("parallel_requests", static_cast<int>(kThreads * kIterationsPerThread));
  RecordProperty("parallel_elapsed_ms", static_cast<int>(ms));

  EXPECT_EQ(failures.load(std::memory_order_relaxed), 0U);
}
