#include <gtest/gtest.h>

#include <boost/beast/http.hpp>

#include "HttpTypes.hpp"
#include "RouteTrie.hpp"

namespace http = boost::beast::http;

class RouteTrieTest : public ::testing::Test {
 protected:
  void SetUp() override { trie_ = std::make_unique<RouteTrie>(); }

  void TearDown() override { trie_.reset(); }

  std::unique_ptr<RouteTrie> trie_;
};

// Тест 1: Добавление простого статического маршрута
TEST_F(RouteTrieTest, AddStaticRoute) {
  bool handler_called = false;

  RouteTrie::Handler handler = [&handler_called](const RequestContext&) {
    handler_called = true;
    return Response{http::status::ok, 11};
  };

  trie_->AddRoute(http::verb::get, "/users", std::move(handler));

  auto [found_handler, params] = trie_->FindRoute(http::verb::get, "/users");

  EXPECT_TRUE(found_handler != nullptr);
  EXPECT_TRUE(params.empty());

  ASSERT_TRUE(found_handler != nullptr);
  // Создаем mock RequestContext
  Request req;
  GeneralParams path_params;
  GeneralParams query_params;
  RequestContext ctx(req, std::move(path_params), std::move(query_params));

  auto _ = found_handler(ctx);
  EXPECT_TRUE(handler_called);
}

// Тест 2: Добавление маршрута с параметром типа int
TEST_F(RouteTrieTest, AddRouteWithIntParameter) {
  RouteTrie::Handler handler = [](const RequestContext&) {
    return Response{http::status::ok, 11};
  };

  trie_->AddRoute(http::verb::get, "/users/{id:int}", std::move(handler));

  auto [found_handler, params] = trie_->FindRoute(http::verb::get, "/users/123");

  EXPECT_TRUE(found_handler != nullptr);
  EXPECT_EQ(params.size(), 1);
  EXPECT_EQ(params["id"], "123");
}

// Тест 3: Добавление маршрута с параметром типа string
TEST_F(RouteTrieTest, AddRouteWithStringParameter) {
  RouteTrie::Handler handler = [](const RequestContext&) {
    return Response{http::status::ok, 11};
  };

  trie_->AddRoute(http::verb::get, "/profile/{username:string}", std::move(handler));

  auto [found_handler, params] = trie_->FindRoute(http::verb::get, "/profile/john_doe");

  EXPECT_TRUE(found_handler != nullptr);
  EXPECT_EQ(params.size(), 1);
  EXPECT_EQ(params["username"], "john_doe");
}

// Тест 4: Приоритет параметров (int > string)
TEST_F(RouteTrieTest, ParameterPriority) {
  RouteTrie::Handler int_handler = [](const RequestContext&) {
    return Response{http::status::ok, 11};
  };

  RouteTrie::Handler string_handler = [](const RequestContext&) {
    return Response{http::status::created, 11};
  };

  // Добавляем два пересекающихся маршрута
  trie_->AddRoute(http::verb::get, "/data/{id:int}", std::move(int_handler));
  trie_->AddRoute(http::verb::get, "/data/{name:string}", std::move(string_handler));

  // Число должно матчиться на int параметр
  auto [handler1, params1] = trie_->FindRoute(http::verb::get, "/data/42");
  EXPECT_TRUE(handler1 != nullptr);

  // Строка должна матчиться на string параметр
  auto [handler2, params2] = trie_->FindRoute(http::verb::get, "/data/product_name");
  EXPECT_TRUE(handler2 != nullptr);

  Request req;
  GeneralParams path_params;
  GeneralParams query_params;
  RequestContext ctx(req, std::move(path_params), std::move(query_params));
  EXPECT_EQ(handler1(ctx).result(), http::status::ok);
  EXPECT_EQ(handler2(ctx).result(), http::status::created);
}

// Тест 5: Вложенные маршруты
TEST_F(RouteTrieTest, NestedRoutes) {
  RouteTrie::Handler handler = [](const RequestContext&) {
    return Response{http::status::ok, 11};
  };

  trie_->AddRoute(http::verb::get, "/api/v1/users/{id:int}/orders", std::move(handler));

  auto [found_handler, params] =
      trie_->FindRoute(http::verb::get, "/api/v1/users/456/orders");

  EXPECT_TRUE(found_handler != nullptr);
  EXPECT_EQ(params.size(), 1);
  EXPECT_EQ(params["id"], "456");
}

// Тест 6: Разные HTTP методы
TEST_F(RouteTrieTest, DifferentHttpMethods) {
  RouteTrie::Handler get_handler = [](const RequestContext&) {
    return Response{http::status::ok, 11};
  };

  RouteTrie::Handler post_handler = [](const RequestContext&) {
    return Response{http::status::created, 11};
  };

  trie_->AddRoute(http::verb::get, "/users", std::move(get_handler));
  trie_->AddRoute(http::verb::post, "/users", std::move(post_handler));

  // GET должен вызвать get_handler
  auto [get_found, get_params] = trie_->FindRoute(http::verb::get, "/users");
  EXPECT_TRUE(get_found != nullptr);

  // POST должен вызвать post_handler
  auto [post_found, post_params] = trie_->FindRoute(http::verb::post, "/users");
  EXPECT_TRUE(post_found != nullptr);

  // Метод PUT не должен найти хендлер
  auto [put_found, put_params] = trie_->FindRoute(http::verb::put, "/users");
  EXPECT_TRUE(put_found == nullptr);
}

// Тест 7: Не найденный маршрут
TEST_F(RouteTrieTest, RouteNotFound) {
  RouteTrie::Handler handler = [](const RequestContext&) {
    return Response{http::status::ok, 11};
  };

  trie_->AddRoute(http::verb::get, "/users", std::move(handler));

  auto [found_handler, params] = trie_->FindRoute(http::verb::get, "/nonexistent");

  EXPECT_TRUE(found_handler == nullptr);
  EXPECT_TRUE(params.empty());
}

// Тест 8: Неверный метод для существующего пути
TEST_F(RouteTrieTest, WrongMethodForPath) {
  RouteTrie::Handler handler = [](const RequestContext&) {
    return Response{http::status::ok, 11};
  };

  trie_->AddRoute(http::verb::get, "/users", std::move(handler));

  // POST там, где только GET
  auto [found_handler, params] = trie_->FindRoute(http::verb::post, "/users");

  EXPECT_TRUE(found_handler == nullptr);
}

// Тест 9: Параметр float
TEST_F(RouteTrieTest, FloatParameter) {
  RouteTrie::Handler handler = [](const RequestContext&) {
    return Response{http::status::ok, 11};
  };

  trie_->AddRoute(http::verb::get, "/price/{amount:float}", std::move(handler));

  // Должен матчиться float
  auto [found_handler, params] = trie_->FindRoute(http::verb::get, "/price/99.99");

  EXPECT_TRUE(found_handler != nullptr);
  EXPECT_EQ(params["amount"], "99.99");

  // Не должен матчиться на не-float
  auto [found_handler2, params2] = trie_->FindRoute(http::verb::get, "/price/abc");
  EXPECT_TRUE(found_handler2 == nullptr);
}

// Тест 10: Получение всех маршрутов
TEST_F(RouteTrieTest, GetAllRoutes) {
  trie_->AddRoute(http::verb::get, "/users",
                  [](const RequestContext&) { return Response{http::status::ok, 11}; });
  trie_->AddRoute(http::verb::post, "/users",
                  [](const RequestContext&) { return Response{http::status::ok, 11}; });
  trie_->AddRoute(http::verb::get, "/users/{id:int}",
                  [](const RequestContext&) { return Response{http::status::ok, 11}; });
  trie_->AddRoute(http::verb::get, "/orders/{order_id:string}/items",
                  [](const RequestContext&) { return Response{http::status::ok, 11}; });

  auto routes = trie_->GetAllRoutes();

  EXPECT_EQ(routes.size(), 4);

  // Проверяем что все маршруты присутствуют
  std::set<std::string> routes_set(routes.begin(), routes.end());

  EXPECT_TRUE(routes_set.contains("GET /users"));
  EXPECT_TRUE(routes_set.contains("POST /users"));
  EXPECT_TRUE(routes_set.contains("GET /users/{id:int}"));
  EXPECT_TRUE(routes_set.contains("GET /orders/{order_id:string}/items"));
}

// Тест 11: Специальные символы в статических путях
TEST_F(RouteTrieTest, SpecialCharactersInStaticPath) {
  // Query параметры не должны быть частью пути маршрута
  trie_->AddRoute(http::verb::get, "/api/v1.0/users",
                  [](const RequestContext&) { return Response{http::status::ok, 11}; });
  trie_->AddRoute(http::verb::get, "/user-name/profile",
                  [](const RequestContext&) { return Response{http::status::ok, 11}; });
  trie_->AddRoute(http::verb::get, "/search",
                  [](const RequestContext&) { return Response{http::status::ok, 11}; });

  auto [found1, params1] = trie_->FindRoute(http::verb::get, "/api/v1.0/users");
  EXPECT_TRUE(found1 != nullptr);

  auto [found2, params2] = trie_->FindRoute(http::verb::get, "/user-name/profile");
  EXPECT_TRUE(found2 != nullptr);

  // Теперь должно работать
  auto [found3, params3] = trie_->FindRoute(http::verb::get, "/search");
  EXPECT_TRUE(found3 != nullptr);
}

// Тест 12: Конфликт статического и параметризованного пути
TEST_F(RouteTrieTest, StaticVsParametricConflict) {
  RouteTrie::Handler static_handler = [](const RequestContext&) {
    return Response{http::status::ok, 11};
  };

  RouteTrie::Handler param_handler = [](const RequestContext&) {
    return Response{http::status::created, 11};
  };

  trie_->AddRoute(http::verb::get, "/users/new", std::move(static_handler));
  trie_->AddRoute(http::verb::get, "/users/{id:int}", std::move(param_handler));

  // Должен найти статический маршрут
  auto [handler1, params1] = trie_->FindRoute(http::verb::get, "/users/new");
  EXPECT_TRUE(handler1 != nullptr);
  EXPECT_TRUE(params1.empty());

  // Должен найти параметризованный маршрут
  auto [handler2, params2] = trie_->FindRoute(http::verb::get, "/users/123");
  EXPECT_TRUE(handler2 != nullptr);
  EXPECT_EQ(params2["id"], "123");
}

// Тест 13: Trailing slashes
TEST_F(RouteTrieTest, TrailingSlashes) {
  RouteTrie::Handler handler = [](const RequestContext&) {
    return Response{http::status::ok, 11};
  };

  trie_->AddRoute(http::verb::get, "/users", std::move(handler));

  // Оба варианта должны работать (нормализация путей)
  auto [found1, params1] = trie_->FindRoute(http::verb::get, "/users");
  auto [found2, params2] = trie_->FindRoute(http::verb::get, "/users/");

  EXPECT_TRUE(found1 != nullptr);
  EXPECT_TRUE(found2 != nullptr);
}

// Тест 14: Пустой путь
TEST_F(RouteTrieTest, EmptyPath) {
  trie_->AddRoute(http::verb::get, "",
                  [](const RequestContext&) { return Response{http::status::ok, 11}; });
  trie_->AddRoute(http::verb::get, "/",
                  [](const RequestContext&) { return Response{http::status::ok, 11}; });

  auto [found1, params1] = trie_->FindRoute(http::verb::get, "");
  auto [found2, params2] = trie_->FindRoute(http::verb::get, "/");

  EXPECT_TRUE(found1 != nullptr);
  EXPECT_TRUE(found2 != nullptr);
}

// Тест 15: Несколько параметров в пути
TEST_F(RouteTrieTest, MultipleParameters) {
  RouteTrie::Handler handler = [](const RequestContext&) {
    return Response{http::status::ok, 11};
  };

  trie_->AddRoute(http::verb::get, "/users/{user_id:int}/orders/{order_id:string}",
                  std::move(handler));

  auto [found_handler, params] =
      trie_->FindRoute(http::verb::get, "/users/123/orders/abc-xyz");

  EXPECT_TRUE(found_handler != nullptr);
  EXPECT_EQ(params.size(), 2);
  EXPECT_EQ(params["user_id"], "123");
  EXPECT_EQ(params["order_id"], "abc-xyz");
}

// Тест 16: Неправильный тип параметра
TEST_F(RouteTrieTest, InvalidParameterType) {
  RouteTrie::Handler handler = [](const RequestContext&) {
    return Response{http::status::ok, 11};
  };

  // Неизвестный тип должен бросить исключение
  EXPECT_THROW(
      trie_->AddRoute(http::verb::get, "/users/{id:unknown_type}", std::move(handler)),
      std::runtime_error);
}

// Тест 17: Конфликт одинаковых matcher с разными именами параметров
TEST_F(RouteTrieTest, AmbiguousParameterNamesWithSameTypeThrows) {
  RouteTrie::Handler handler = [](const RequestContext&) {
    return Response{http::status::ok, 11};
  };

  trie_->AddRoute(http::verb::get, "/users/{id:int}", std::move(handler));

  EXPECT_THROW(trie_->AddRoute(
                   http::verb::get, "/users/{user_id:int}",
                   [](const RequestContext&) { return Response{http::status::ok, 11}; }),
               std::runtime_error);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
