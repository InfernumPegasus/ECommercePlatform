#include <gtest/gtest.h>

#include <string>
#include <unordered_map>

#include "TypedParams.hpp"
#include "ValueParser.hpp"

class RouteParsingIntegrationTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Симуляция параметров из маршрута
    route_params_ = {
        {"user_id", "123"},       {"order_id", "456"},   {"price", "99.99"},
        {"is_active", "true"},    {"quantity", "10"},    {"discount", "0.15"},
        {"username", "john_doe"}, {"is_admin", "false"}, {"page", "1"},
        {"limit", "20"}};

    typed_params_ = std::make_unique<TypedParams>(route_params_);
  }

  void TearDown() override { typed_params_.reset(); }

  std::unordered_map<std::string, std::string> route_params_;
  std::unique_ptr<TypedParams> typed_params_;
};

// Интеграционные тесты для типичных сценариев маршрутов
TEST_F(RouteParsingIntegrationTest, UserProfileRoute) {
  // /users/{user_id:int}/profile
  auto user_id = typed_params_->TryGet<int64_t>("user_id");
  EXPECT_TRUE(user_id.has_value());
  EXPECT_EQ(*user_id, 123LL);

  // Проверяем Required метод
  EXPECT_EQ(typed_params_->Required<int64_t>("user_id"), 123LL);
}

TEST_F(RouteParsingIntegrationTest, OrderDetailsRoute) {
  // /orders/{order_id:int}
  auto order_id = typed_params_->TryGet<int>("order_id");
  EXPECT_TRUE(order_id.has_value());
  EXPECT_EQ(*order_id, 456);

  auto price = typed_params_->TryGet<double>("price");
  EXPECT_TRUE(price.has_value());
  EXPECT_DOUBLE_EQ(*price, 99.99);

  auto quantity = typed_params_->TryGet<int>("quantity");
  EXPECT_TRUE(quantity.has_value());
  EXPECT_EQ(*quantity, 10);
}

TEST_F(RouteParsingIntegrationTest, BooleanFlagsInRoutes) {
  // /users?is_active=true&is_admin=false
  auto is_active = typed_params_->TryGet<bool>("is_active");
  EXPECT_TRUE(is_active.has_value());
  EXPECT_EQ(*is_active, true);

  auto is_admin = typed_params_->TryGet<bool>("is_admin");
  EXPECT_TRUE(is_admin.has_value());
  EXPECT_EQ(*is_admin, false);

  // Проверяем разные представления булевых значений
  route_params_["bool_as_int"] = "1";
  route_params_["bool_as_int_zero"] = "0";
  route_params_["bool_upper"] = "TRUE";
  route_params_["bool_lower"] = "false";

  TypedParams bool_params(route_params_);

  EXPECT_EQ(bool_params.Required<bool>("bool_as_int"), true);
  EXPECT_EQ(bool_params.Required<bool>("bool_as_int_zero"), false);
  EXPECT_EQ(bool_params.Required<bool>("bool_upper"), true);
  EXPECT_EQ(bool_params.Required<bool>("bool_lower"), false);
}

TEST_F(RouteParsingIntegrationTest, PaginationParameters) {
  // /items?page=1&limit=20
  auto page = typed_params_->TryGet<int>("page");
  EXPECT_TRUE(page.has_value());
  EXPECT_EQ(*page, 1);

  auto limit = typed_params_->TryGet<int>("limit");
  EXPECT_TRUE(limit.has_value());
  EXPECT_EQ(*limit, 20);

  // Проверяем дефолтные значения через value_or
  auto offset = typed_params_->TryGet<int>("offset").value_or(0);
  EXPECT_EQ(offset, 0);  // offset не существует, должен вернуть 0

  // Проверяем Required с несуществующим параметром
  EXPECT_THROW(typed_params_->Required<int>("offset"), std::runtime_error);
}

TEST_F(RouteParsingIntegrationTest, StringParameters) {
  // /users/{username:string}
  auto username = typed_params_->TryGet<std::string>("username");
  EXPECT_TRUE(username.has_value());
  EXPECT_EQ(*username, "john_doe");

  // string_view тоже должен работать
  auto username_view = typed_params_->TryGet<std::string_view>("username");
  EXPECT_TRUE(username_view.has_value());
  EXPECT_EQ(*username_view, "john_doe");

  // Проверяем, что view возвращает корректное значение
  EXPECT_EQ(*username_view, route_params_["username"]);
}

TEST_F(RouteParsingIntegrationTest, FloatAndDoubleParameters) {
  // /products/{price:float}
  auto price = typed_params_->TryGet<double>("price");
  EXPECT_TRUE(price.has_value());
  EXPECT_DOUBLE_EQ(*price, 99.99);

  auto discount = typed_params_->TryGet<float>("discount");
  EXPECT_TRUE(discount.has_value());
  EXPECT_FLOAT_EQ(*discount, 0.15f);

  // Научная нотация
  route_params_["scientific"] = "1.5e6";
  TypedParams scientific_params(route_params_);

  auto scientific = scientific_params.TryGet<double>("scientific");
  EXPECT_TRUE(scientific.has_value());
  EXPECT_DOUBLE_EQ(*scientific, 1500000.0);
}

TEST_F(RouteParsingIntegrationTest, InvalidRouteParameters) {
  // Симуляция невалидных параметров маршрута
  std::unordered_map<std::string, std::string> invalid_params = {
      {"user_id", "not_a_number"},
      {"price", "invalid_float"},
      {"is_active", "yes"},  // не "true"/"false"
      {"empty", ""},
      {"with_spaces", " 123 "}};

  TypedParams invalid_typed_params(invalid_params);

  // Невалидные преобразования должны возвращать nullopt
  EXPECT_FALSE(invalid_typed_params.TryGet<int>("user_id").has_value());
  EXPECT_FALSE(invalid_typed_params.TryGet<double>("price").has_value());
  EXPECT_FALSE(invalid_typed_params.TryGet<bool>("is_active").has_value());
  EXPECT_FALSE(invalid_typed_params.TryGet<int>("empty").has_value());
  EXPECT_FALSE(invalid_typed_params.TryGet<int>("with_spaces").has_value());

  // Required должен кидать исключения
  EXPECT_THROW(invalid_typed_params.Required<int>("user_id"), std::runtime_error);
  EXPECT_THROW(invalid_typed_params.Required<double>("price"), std::runtime_error);
  EXPECT_THROW(invalid_typed_params.Required<bool>("is_active"), std::runtime_error);
}

TEST_F(RouteParsingIntegrationTest, MixedValidAndInvalidParameters) {
  // Смесь валидных и невалидных параметров
  std::unordered_map<std::string, std::string> mixed_params = {
      {"valid_int", "42"},       {"invalid_int", "abc"}, {"valid_float", "3.14"},
      {"invalid_float", "x.y"},  {"valid_bool", "true"}, {"invalid_bool", "yes"},
      {"valid_string", "hello"}, {"empty_string", ""}};

  TypedParams mixed_typed_params(mixed_params);

  // Валидные должны работать
  EXPECT_EQ(mixed_typed_params.Required<int>("valid_int"), 42);
  EXPECT_DOUBLE_EQ(mixed_typed_params.Required<double>("valid_float"), 3.14);
  EXPECT_EQ(mixed_typed_params.Required<bool>("valid_bool"), true);
  EXPECT_EQ(mixed_typed_params.Required<std::string>("valid_string"), "hello");

  // Невалидные должны возвращать nullopt
  EXPECT_FALSE(mixed_typed_params.TryGet<int>("invalid_int").has_value());
  EXPECT_FALSE(mixed_typed_params.TryGet<double>("invalid_float").has_value());
  EXPECT_FALSE(mixed_typed_params.TryGet<bool>("invalid_bool").has_value());

  // Пустая строка как число - невалидно
  EXPECT_FALSE(mixed_typed_params.TryGet<int>("empty_string").has_value());

  // Пустая строка как строка - валидно
  EXPECT_TRUE(mixed_typed_params.TryGet<std::string>("empty_string").has_value());
  EXPECT_EQ(mixed_typed_params.Required<std::string>("empty_string"), "");
}

TEST_F(RouteParsingIntegrationTest, PerformanceUnderLoad) {
  // Тест производительности при множественных запросах
  const int iterations = 1000;

  for (int i = 0; i < iterations; ++i) {
    // Множественные TryGet
    auto user_id = typed_params_->TryGet<int64_t>("user_id");
    EXPECT_TRUE(user_id.has_value());

    auto price = typed_params_->TryGet<double>("price");
    EXPECT_TRUE(price.has_value());

    auto is_active = typed_params_->TryGet<bool>("is_active");
    EXPECT_TRUE(is_active.has_value());

    auto username = typed_params_->TryGet<std::string>("username");
    EXPECT_TRUE(username.has_value());

    // Множественные Required
    EXPECT_EQ(typed_params_->Required<int>("order_id"), 456);
    EXPECT_EQ(typed_params_->Required<int>("quantity"), 10);
  }
}

TEST_F(RouteParsingIntegrationTest, ErrorMessagesAreInformative) {
  // Проверяем, что сообщения об ошибках содержат полезную информацию
  try {
    typed_params_->Required<int>("nonexistent_param");
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    std::string error = e.what();
    EXPECT_NE(error.find("nonexistent_param"), std::string::npos)
        << "Error message should contain parameter name";
  }

  try {
    typed_params_->Required<int>("username");  // строка вместо числа
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    std::string error = e.what();
    EXPECT_NE(error.find("username"), std::string::npos)
        << "Error message should contain parameter name";
  }
}

TEST_F(RouteParsingIntegrationTest, RealWorldRouteExamples) {
  // Примеры реальных маршрутов из проекта

  // 1. /orders/{order_id:int}/items/{item_id:int}
  std::unordered_map<std::string, std::string> order_item_route = {{"order_id", "1001"},
                                                                   {"item_id", "42"}};
  TypedParams order_item_params(order_item_route);

  EXPECT_EQ(order_item_params.Required<int>("order_id"), 1001);
  EXPECT_EQ(order_item_params.Required<int>("item_id"), 42);

  // 2. /products/{product_id:int}?include_reviews=true&limit=5
  std::unordered_map<std::string, std::string> product_route = {
      {"product_id", "777"}, {"include_reviews", "true"}, {"limit", "5"}};
  TypedParams product_params(product_route);

  EXPECT_EQ(product_params.Required<int>("product_id"), 777);
  EXPECT_EQ(product_params.Required<bool>("include_reviews"), true);
  EXPECT_EQ(product_params.Required<int>("limit"), 5);

  // 3. /search?q=query&page=2&sort=price&descending=true
  std::unordered_map<std::string, std::string> search_route = {
      {"q", "laptop"}, {"page", "2"}, {"sort", "price"}, {"descending", "true"}};
  TypedParams search_params(search_route);

  EXPECT_EQ(search_params.Required<std::string>("q"), "laptop");
  EXPECT_EQ(search_params.Required<int>("page"), 2);
  EXPECT_EQ(search_params.Required<std::string>("sort"), "price");
  EXPECT_EQ(search_params.Required<bool>("descending"), true);

  // 4. Необязательные параметры
  auto optional_param = search_params.TryGet<int>("rating_min");
  EXPECT_FALSE(optional_param.has_value());  // не существует

  int default_rating = search_params.TryGet<int>("rating_min").value_or(0);
  EXPECT_EQ(default_rating, 0);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
