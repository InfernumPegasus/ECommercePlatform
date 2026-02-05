#include <gtest/gtest.h>

#include <boost/beast/http.hpp>

#include "IController.hpp"
#include "RouteDSL.hpp"
#include "Router.hpp"

namespace http = boost::beast::http;

// Mock контроллер для тестирования
class MockController : public IController<MockController> {
 public:
  static constexpr auto Routes() {
    using R = route_dsl::RouteBuilder<MockController>;

    return R::WithBase("/api", R::GET("/users", &MockController::GetUsers),
                       R::GET("/users/{id:int}", &MockController::GetUserById),
                       R::POST("/users", &MockController::CreateUser),
                       R::PUT("/users/{id:int}", &MockController::UpdateUser),
                       R::DEL("/users/{id:int}", &MockController::DeleteUser));
  }

  // Mock методы
  Response GetUsers(const RequestContext&) {
    ++call_count_["GetUsers"];
    return CreateResponse("GetUsers");
  }

  Response GetUserById(const RequestContext&) {
    ++call_count_["GetUserById"];
    return CreateResponse("GetUserById");
  }

  Response CreateUser(const RequestContext&) {
    ++call_count_["CreateUser"];
    return CreateResponse("CreateUser");
  }

  Response UpdateUser(const RequestContext&) {
    ++call_count_["UpdateUser"];
    return CreateResponse("UpdateUser");
  }

  Response DeleteUser(const RequestContext&) {
    ++call_count_["DeleteUser"];
    return CreateResponse("DeleteUser");
  }

  // Вспомогательные методы для тестов
  int GetCallCount(const std::string& method) const {
    auto it = call_count_.find(method);
    return it != call_count_.end() ? it->second : 0;
  }

  void ResetCallCount() { call_count_.clear(); }

 private:
  Response CreateResponse(const std::string& handler_name) {
    Response res{http::status::ok, 11};
    res.set(http::field::content_type, "text/plain");
    res.body() = "Handler: " + handler_name;
    res.prepare_payload();
    return res;
  }

  std::unordered_map<std::string, int> call_count_;
};

class RouterTest : public ::testing::Test {
 protected:
  void SetUp() override {
    router_ = std::make_unique<Router>();
    controller_ = std::make_unique<MockController>();

    // Регистрируем маршруты контроллера
    controller_->RegisterRoutes(*router_);
  }

  void TearDown() override {
    controller_.reset();
    router_.reset();
  }

  std::unique_ptr<Router> router_;
  std::unique_ptr<MockController> controller_;

  // Вспомогательный метод для создания запроса
  Request CreateRequest(http::verb method, const std::string& target) {
    Request req;
    req.method(method);
    req.target(target);
    req.version(11);
    req.set(http::field::host, "localhost");
    req.keep_alive(true);
    return req;
  }
};

// Тест 1: Базовый роутинг статических путей
TEST_F(RouterTest, BasicStaticRouting) {
  auto req = CreateRequest(http::verb::get, "/api/users");
  auto response = router_->Route(req);

  EXPECT_EQ(controller_->GetCallCount("GetUsers"), 1);
  EXPECT_EQ(response.result(), http::status::ok);
  EXPECT_EQ(response.body(), "Handler: GetUsers");
}

// Тест 2: Роутинг с параметрами
TEST_F(RouterTest, RoutingWithParameters) {
  auto req = CreateRequest(http::verb::get, "/api/users/123");
  auto response = router_->Route(req);

  EXPECT_EQ(controller_->GetCallCount("GetUserById"), 1);
  EXPECT_EQ(response.result(), http::status::ok);
  EXPECT_EQ(response.body(), "Handler: GetUserById");
}

// Тест 3: Разные HTTP методы
TEST_F(RouterTest, DifferentHttpMethods) {
  // GET
  auto get_req = CreateRequest(http::verb::get, "/api/users");
  router_->Route(get_req);
  EXPECT_EQ(controller_->GetCallCount("GetUsers"), 1);

  // POST
  controller_->ResetCallCount();
  auto post_req = CreateRequest(http::verb::post, "/api/users");
  router_->Route(post_req);
  EXPECT_EQ(controller_->GetCallCount("CreateUser"), 1);

  // PUT
  controller_->ResetCallCount();
  auto put_req = CreateRequest(http::verb::put, "/api/users/456");
  router_->Route(put_req);
  EXPECT_EQ(controller_->GetCallCount("UpdateUser"), 1);

  // DELETE
  controller_->ResetCallCount();
  auto delete_req = CreateRequest(http::verb::delete_, "/api/users/789");
  router_->Route(delete_req);
  EXPECT_EQ(controller_->GetCallCount("DeleteUser"), 1);
}

// Тест 4: Не найденный маршрут
TEST_F(RouterTest, RouteNotFound) {
  // Несуществующий путь
  auto req = CreateRequest(http::verb::get, "/api/nonexistent");
  auto response = router_->Route(req);

  EXPECT_EQ(response.result(), http::status::not_found);
  EXPECT_EQ(controller_->GetCallCount("GetUsers"), 0);

  // Неправильный метод для существующего пути
  auto req2 = CreateRequest(http::verb::patch, "/api/users");
  auto response2 = router_->Route(req2);

  EXPECT_EQ(response2.result(), http::status::not_found);
}

// Тест 5: Query параметры передаются
TEST_F(RouterTest, QueryParametersPassed) {
  // Создаем запрос с query параметрами
  Request req;
  req.method(http::verb::get);
  req.target("/api/users?limit=10&offset=20&active=true");
  req.version(11);
  req.set(http::field::host, "localhost");

  auto response = router_->Route(req);

  EXPECT_EQ(controller_->GetCallCount("GetUsers"), 1);
  EXPECT_EQ(response.result(), http::status::ok);
}

// Тест 6: Неправильный путь параметра
TEST_F(RouterTest, InvalidParameterPath) {
  // "abc" не является int, поэтому не должно матчиться
  auto req = CreateRequest(http::verb::get, "/api/users/abc");
  auto response = router_->Route(req);

  EXPECT_EQ(response.result(), http::status::not_found);
  EXPECT_EQ(controller_->GetCallCount("GetUserById"), 0);
}

// Тест 7: Неверный формат пути
TEST_F(RouterTest, InvalidPathFormat) {
  Request req;
  req.method(http::verb::get);
  req.target("invalid path with spaces");  // Некорректный target
  req.version(11);
  req.set(http::field::host, "localhost");

  auto response = router_->Route(req);

  EXPECT_EQ(response.result(), http::status::bad_request);
}

// Тест 8: Trailing slashes
TEST_F(RouterTest, TrailingSlashes) {
  // Оба варианта должны работать
  auto req1 = CreateRequest(http::verb::get, "/api/users");
  auto response1 = router_->Route(req1);
  EXPECT_EQ(response1.result(), http::status::ok);

  auto req2 = CreateRequest(http::verb::get, "/api/users/");
  auto response2 = router_->Route(req2);
  EXPECT_EQ(response2.result(), http::status::ok);
}

// Тест 9: Путь с несколькими параметрами (если бы был такой маршрут)
TEST_F(RouterTest, PathWithMultipleParameters) {
  // Для этого теста нужно добавить соответствующий маршрут
  class MultiParamController : public IController<MultiParamController> {
   public:
    static auto Routes() {
      using R = route_dsl::RouteBuilder<MultiParamController>;
      return R::Routes(R::GET("/users/{user_id:int}/orders/{order_id:string}",
                              &MultiParamController::GetOrder));
    }

    Response GetOrder(const RequestContext& ctx) {
      auto user_id = ctx.GetPathParameters().Required<int>("user_id");
      auto order_id = ctx.GetPathParameters().Required<std::string>("order_id");

      Response res{http::status::ok, 11};
      res.set(http::field::content_type, "text/plain");
      res.body() = "User: " + std::to_string(user_id) + ", Order: " + order_id;
      res.prepare_payload();
      return res;
    }
  };

  MultiParamController mp_controller;
  Router test_router;
  mp_controller.RegisterRoutes(test_router);

  auto req = CreateRequest(http::verb::get, "/users/123/orders/order-abc");
  auto response = test_router.Route(req);

  EXPECT_EQ(response.result(), http::status::ok);
  EXPECT_EQ(response.body(), "User: 123, Order: order-abc");
}

// Тест 10: Порядок регистрации маршрутов
TEST_F(RouterTest, RouteRegistrationOrder) {
  // Тест на то, что порядок регистрации не ломает поиск
  Router test_router;

  // Создаем простые хендлеры
  bool handler1_called = false;
  bool handler2_called = false;

  class TestController : public IController<TestController> {
   public:
    TestController(bool& h1, bool& h2) : handler1_called_(h1), handler2_called_(h2) {}

    static constexpr auto Routes() {
      using R = route_dsl::RouteBuilder<TestController>;
      return R::Routes(R::GET("/users/{id:int}", &TestController::Handler1),
                       R::GET("/users/new", &TestController::Handler2));
    }

    Response Handler1(const RequestContext&) {
      handler1_called_ = true;
      return Response{http::status::ok, 11};
    }

    Response Handler2(const RequestContext&) {
      handler2_called_ = true;
      return Response{http::status::ok, 11};
    }

   private:
    bool& handler1_called_;
    bool& handler2_called_;
  };

  TestController test_controller(handler1_called, handler2_called);
  test_controller.RegisterRoutes(test_router);

  // Проверяем что статический путь "/users/new" не матчится как параметр
  auto req1 = CreateRequest(http::verb::get, "/users/new");
  auto response1 = test_router.Route(req1);

  EXPECT_TRUE(handler2_called);
  EXPECT_FALSE(handler1_called);

  // Проверяем что параметризованный путь работает
  auto req2 = CreateRequest(http::verb::get, "/users/123");
  auto response2 = test_router.Route(req2);

  EXPECT_TRUE(handler1_called);
}

// Тест 11: Печать всех маршрутов (debug функциональность)
TEST_F(RouterTest, PrintAllRoutes) {
  // Этот тест в основном проверяет что метод не падает
  testing::internal::CaptureStdout();
  router_->PrintAllRoutes();
  std::string output = testing::internal::GetCapturedStdout();

  // Проверяем что в выводе есть наши маршруты
  EXPECT_TRUE(output.find("GET") != std::string::npos);
  EXPECT_TRUE(output.find("/api") != std::string::npos);
}

// Тест 12: Path параметры разных типов
TEST_F(RouterTest, DifferentParameterTypes) {
  class TypeTestController : public IController<TypeTestController> {
   public:
    static constexpr auto Routes() {
      using R = route_dsl::RouteBuilder<TypeTestController>;
      return R::Routes(R::GET("/test/{id:int}", &TypeTestController::HandleInt),
                       R::GET("/test/{value:float}", &TypeTestController::HandleFloat),
                       R::GET("/test/{name:string}", &TypeTestController::HandleString));
    }

    Response HandleInt(const RequestContext& ctx) {
      auto id = ctx.GetPathParameters().Required<int>("id");
      Response res{http::status::ok, 11};
      res.body() = "int: " + std::to_string(id);
      return res;
    }

    Response HandleFloat(const RequestContext& ctx) {
      auto value = ctx.GetPathParameters().Required<double>("value");
      Response res{http::status::ok, 11};
      res.body() = "float: " + std::to_string(value);
      return res;
    }

    Response HandleString(const RequestContext& ctx) {
      auto name = ctx.GetPathParameters().Required<std::string>("name");
      Response res{http::status::ok, 11};
      res.body() = "string: " + name;
      return res;
    }
  };

  TypeTestController type_controller;
  Router test_router;
  type_controller.RegisterRoutes(test_router);

  // Тестируем int
  auto req1 = CreateRequest(http::verb::get, "/test/42");
  auto resp1 = test_router.Route(req1);
  EXPECT_EQ(resp1.body(), "int: 42");

  // Тестируем float
  auto req2 = CreateRequest(http::verb::get, "/test/3.14");
  auto resp2 = test_router.Route(req2);
  EXPECT_EQ(resp2.body(), "float: 3.140000");

  // Тестируем string
  auto req3 = CreateRequest(http::verb::get, "/test/hello");
  auto resp3 = test_router.Route(req3);
  EXPECT_EQ(resp3.body(), "string: hello");
}

// Тест 13: Контроллер без маршрутов
TEST_F(RouterTest, ControllerWithoutRoutes) {
  class EmptyController : public IController<EmptyController> {
   public:
    static auto Routes() {
      return std::array<typename IController<EmptyController>::RouteDescription, 0>{};
    }
  };

  EmptyController empty_controller;
  Router test_router;

  // Не должно быть ошибок при регистрации пустого контроллера
  EXPECT_NO_THROW(empty_controller.RegisterRoutes(test_router));

  // Маршрутов не должно быть
  auto req = CreateRequest(http::verb::get, "/anything");
  auto response = test_router.Route(req);
  EXPECT_EQ(response.result(), http::status::not_found);
}

// Тест 14: Keep-alive заголовок сохраняется
TEST_F(RouterTest, KeepAliveHeaderPreserved) {
  auto req = CreateRequest(http::verb::get, "/api/users");
  req.keep_alive(true);

  auto response = router_->Route(req);

  EXPECT_TRUE(response.keep_alive());
  EXPECT_EQ(response.result(), http::status::ok);
}

// Тест 15: Body запроса передается в обработчик
TEST_F(RouterTest, RequestBodyPassedToHandler) {
  class BodyTestController : public IController<BodyTestController> {
   public:
    static constexpr auto Routes() {
      using R = route_dsl::RouteBuilder<BodyTestController>;
      return R::Routes(R::POST("/echo", &BodyTestController::Echo));
    }

    Response Echo(const RequestContext& ctx) {
      const auto& req = ctx.GetRequest();
      Response res{http::status::ok, 11};
      res.set(http::field::content_type, "text/plain");
      res.body() = req.body();
      res.prepare_payload();
      return res;
    }
  };

  BodyTestController body_controller;
  Router test_router;
  body_controller.RegisterRoutes(test_router);

  Request req;
  req.method(http::verb::post);
  req.target("/echo");
  req.version(11);
  req.set(http::field::host, "localhost");
  req.set(http::field::content_type, "text/plain");
  req.body() = "Test request body";
  req.prepare_payload();

  auto response = test_router.Route(req);

  EXPECT_EQ(response.result(), http::status::ok);
  EXPECT_EQ(response.body(), "Test request body");
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
