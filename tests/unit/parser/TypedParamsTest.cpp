// begin tests/unit/parser/TypedParamsTest.cpp
#include <gtest/gtest.h>

#include <string>
#include <unordered_map>

#include "TypedParams.hpp"

class TypedParamsTest : public ::testing::Test {
 protected:
  void SetUp() override {
    // Подготавливаем тестовые данные
    test_data_ = {{"id", "123"},
                  {"name", "John Doe"},
                  {"price", "99.99"},
                  {"active", "true"},
                  {"count", "42"},
                  {"empty", ""},
                  {"negative", "-1"},
                  {"float_scientific", "1.23e4"},
                  {"bool_true_upper", "TRUE"},
                  {"bool_false_upper", "FALSE"}};

    typed_params_ = std::make_unique<TypedParams>(test_data_);
  }

  void TearDown() override { typed_params_.reset(); }

  std::unordered_map<std::string, std::string> test_data_;
  std::unique_ptr<TypedParams> typed_params_;
};

// Тесты для TryGet с валидными значениями
TEST_F(TypedParamsTest, TryGetValidValues) {
  // Целое число
  auto id = typed_params_->TryGet<int>("id");
  EXPECT_TRUE(id.has_value());
  EXPECT_EQ(*id, 123);

  // Строка
  auto name = typed_params_->TryGet<std::string>("name");
  EXPECT_TRUE(name.has_value());
  EXPECT_EQ(*name, "John Doe");

  // Число с плавающей точкой
  auto price = typed_params_->TryGet<double>("price");
  EXPECT_TRUE(price.has_value());
  EXPECT_DOUBLE_EQ(*price, 99.99);

  // Булево значение
  auto active = typed_params_->TryGet<bool>("active");
  EXPECT_TRUE(active.has_value());
  EXPECT_EQ(*active, true);

  // Разные целочисленные типы
  auto count_int32 = typed_params_->TryGet<int32_t>("count");
  EXPECT_TRUE(count_int32.has_value());
  EXPECT_EQ(*count_int32, 42);

  auto count_int64 = typed_params_->TryGet<int64_t>("count");
  EXPECT_TRUE(count_int64.has_value());
  EXPECT_EQ(*count_int64, 42LL);

  // Отрицательное число
  auto negative = typed_params_->TryGet<int>("negative");
  EXPECT_TRUE(negative.has_value());
  EXPECT_EQ(*negative, -1);

  // Научная нотация
  auto scientific = typed_params_->TryGet<double>("float_scientific");
  EXPECT_TRUE(scientific.has_value());
  EXPECT_DOUBLE_EQ(*scientific, 12300.0);

  // Булево в верхнем регистре
  auto bool_upper = typed_params_->TryGet<bool>("bool_true_upper");
  EXPECT_TRUE(bool_upper.has_value());
  EXPECT_EQ(*bool_upper, true);

  auto bool_false_upper = typed_params_->TryGet<bool>("bool_false_upper");
  EXPECT_TRUE(bool_false_upper.has_value());
  EXPECT_EQ(*bool_false_upper, false);
}

// Тесты для TryGet с несуществующими ключами
TEST_F(TypedParamsTest, TryGetNonExistentKey) {
  EXPECT_FALSE(typed_params_->TryGet<int>("nonexistent").has_value());
  EXPECT_FALSE(typed_params_->TryGet<std::string>("unknown").has_value());
  EXPECT_FALSE(typed_params_->TryGet<double>("missing").has_value());
  EXPECT_FALSE(typed_params_->TryGet<bool>("not_found").has_value());
}

// Тесты для TryGet с невалидными преобразованиями
TEST_F(TypedParamsTest, TryGetInvalidConversions) {
  // Попытка парсить строку как число
  auto name_as_int = typed_params_->TryGet<int>("name");
  EXPECT_FALSE(name_as_int.has_value());

  // Попытка парсить float как int
  auto price_as_int = typed_params_->TryGet<int>("price");
  EXPECT_FALSE(price_as_int.has_value());

  // Пустая строка как число
  auto empty_as_int = typed_params_->TryGet<int>("empty");
  EXPECT_FALSE(empty_as_int.has_value());

  // Пустая строка как bool
  auto empty_as_bool = typed_params_->TryGet<bool>("empty");
  EXPECT_FALSE(empty_as_bool.has_value());

  // Строка как bool (не "true"/"false")
  auto name_as_bool = typed_params_->TryGet<bool>("name");
  EXPECT_FALSE(name_as_bool.has_value());
}

// Тесты для Required с валидными значениями
TEST_F(TypedParamsTest, RequiredValidValues) {
  EXPECT_EQ(typed_params_->Required<int>("id"), 123);
  EXPECT_EQ(typed_params_->Required<std::string>("name"), "John Doe");
  EXPECT_DOUBLE_EQ(typed_params_->Required<double>("price"), 99.99);
  EXPECT_EQ(typed_params_->Required<bool>("active"), true);
  EXPECT_EQ(typed_params_->Required<int64_t>("count"), 42LL);
}

// Тесты для Required с несуществующими ключами (должно кидать исключение)
TEST_F(TypedParamsTest, RequiredNonExistentKeyThrows) {
  EXPECT_THROW(typed_params_->Required<int>("nonexistent"), std::runtime_error);
  EXPECT_THROW(typed_params_->Required<std::string>("unknown"), std::runtime_error);
  EXPECT_THROW(typed_params_->Required<double>("missing"), std::runtime_error);
  EXPECT_THROW(typed_params_->Required<bool>("not_found"), std::runtime_error);
}

// Тесты для Required с невалидными преобразованиями (должно кидать исключение)
TEST_F(TypedParamsTest, RequiredInvalidConversionThrows) {
  EXPECT_THROW(typed_params_->Required<int>("name"),
               std::runtime_error);  // строка как int
  EXPECT_THROW(typed_params_->Required<bool>("name"),
               std::runtime_error);  // строка как bool
  EXPECT_THROW(typed_params_->Required<int>("price"),
               std::runtime_error);  // float как int
  EXPECT_THROW(typed_params_->Required<int>("empty"),
               std::runtime_error);  // пустая строка как int
}

// Тесты для GetRawParams
TEST_F(TypedParamsTest, GetRawParams) {
  const auto& raw_params = typed_params_->GetRawParams();

  EXPECT_EQ(raw_params.size(), test_data_.size());

  // Проверяем, что все значения совпадают
  for (const auto& [key, expected_value] : test_data_) {
    auto it = raw_params.find(key);
    EXPECT_NE(it, raw_params.end());
    EXPECT_EQ(it->second, expected_value);
  }

  // TypedParams хранит собственную копию параметров
  EXPECT_NE(&raw_params, &test_data_);
}

// Тесты для работы с пустыми параметрами
TEST_F(TypedParamsTest, EmptyParams) {
  std::unordered_map<std::string, std::string> empty_data;
  TypedParams empty_params(empty_data);

  EXPECT_FALSE(empty_params.TryGet<int>("any").has_value());
  EXPECT_FALSE(empty_params.TryGet<std::string>("any").has_value());

  EXPECT_THROW(empty_params.Required<int>("any"), std::runtime_error);

  const auto& raw_empty = empty_params.GetRawParams();
  EXPECT_TRUE(raw_empty.empty());
}

// Тесты для множественных вызовов
TEST_F(TypedParamsTest, MultipleCalls) {
  // Многократные вызовы TryGet должны возвращать одинаковые результаты
  auto first_call = typed_params_->TryGet<int>("id");
  auto second_call = typed_params_->TryGet<int>("id");

  EXPECT_TRUE(first_call.has_value());
  EXPECT_TRUE(second_call.has_value());
  EXPECT_EQ(*first_call, *second_call);

  // Многократные вызовы Required должны возвращать одинаковые значения
  EXPECT_EQ(typed_params_->Required<std::string>("name"),
            typed_params_->Required<std::string>("name"));
}

// Тесты для разных типов строк
TEST_F(TypedParamsTest, DifferentStringTypes) {
  // std::string
  auto name_str = typed_params_->TryGet<std::string>("name");
  EXPECT_TRUE(name_str.has_value());
  EXPECT_EQ(*name_str, "John Doe");

  // std::string_view
  auto name_sv = typed_params_->TryGet<std::string_view>("name");
  EXPECT_TRUE(name_sv.has_value());
  EXPECT_EQ(*name_sv, "John Doe");

  // Проверяем, что string_view возвращает корректное значение
  EXPECT_EQ(*name_sv, test_data_["name"]);
}

// Тесты для специальных числовых значений
TEST_F(TypedParamsTest, SpecialNumericValues) {
  // Добавляем специальные значения
  test_data_["max_int32"] = "2147483647";
  test_data_["min_int32"] = "-2147483648";
  test_data_["max_uint32"] = "4294967295";
  test_data_["zero"] = "0";
  test_data_["negative_zero"] = "-0";

  TypedParams special_params(test_data_);

  EXPECT_EQ(special_params.Required<int32_t>("max_int32"), 2147483647);
  EXPECT_EQ(special_params.Required<int32_t>("min_int32"), -2147483648);
  EXPECT_EQ(special_params.Required<uint32_t>("max_uint32"), 4294967295U);
  EXPECT_EQ(special_params.Required<int>("zero"), 0);
  EXPECT_EQ(special_params.Required<int>("negative_zero"), 0);
}

// Тесты для булевых значений в разных регистрах
TEST_F(TypedParamsTest, BooleanCaseVariations) {
  test_data_["bool_variations"] = "TrUe";  // смешанный регистр
  TypedParams bool_params(test_data_);

  // Смешанный регистр должен работать
  auto mixed_case = bool_params.TryGet<bool>("bool_variations");
  EXPECT_TRUE(mixed_case.has_value());
  EXPECT_EQ(*mixed_case, true);
}

// Тесты для обработки ошибок в Required
TEST_F(TypedParamsTest, RequiredErrorMessage) {
  try {
    typed_params_->Required<int>("name");  // "name" это строка, не число
    FAIL() << "Expected std::runtime_error";
  } catch (const std::runtime_error& e) {
    // Проверяем, что сообщение содержит название параметра
    std::string error_msg = e.what();
    EXPECT_TRUE(error_msg.find("name") != std::string::npos)
        << "Error message should contain parameter name: " << error_msg;
    EXPECT_TRUE(error_msg.find("Missing or invalid") != std::string::npos)
        << "Error message should indicate missing/invalid parameter: " << error_msg;
  }
}

// Тесты для неизменяемости данных
TEST_F(TypedParamsTest, DataImmutability) {
  // Попытка изменить оригинальные данные после создания TypedParams
  test_data_["new_key"] = "new_value";

  // TypedParams хранит snapshot, изменения исходной map не должны влиять
  auto new_value = typed_params_->TryGet<std::string>("new_key");
  EXPECT_FALSE(new_value.has_value());
}

// Тесты для производительности
TEST_F(TypedParamsTest, PerformanceMultipleAccesses) {
  // Многократный доступ к одним и тем же параметрам
  for (int i = 0; i < 1000; ++i) {
    auto id = typed_params_->TryGet<int>("id");
    EXPECT_TRUE(id.has_value());
    EXPECT_EQ(*id, 123);

    auto name = typed_params_->TryGet<std::string>("name");
    EXPECT_TRUE(name.has_value());
    EXPECT_EQ(*name, "John Doe");
  }
}

// Тесты для граничных случаев с типами
TEST_F(TypedParamsTest, TypeBoundaryCases) {
  // Добавляем граничные значения
  test_data_["int8_max"] = "127";
  test_data_["int8_min"] = "-128";
  test_data_["uint8_max"] = "255";
  test_data_["uint8_min"] = "0";

  TypedParams boundary_params(test_data_);

  // Проверяем граничные значения
  EXPECT_EQ(boundary_params.Required<int8_t>("int8_max"), 127);
  EXPECT_EQ(boundary_params.Required<int8_t>("int8_min"), -128);
  EXPECT_EQ(boundary_params.Required<uint8_t>("uint8_max"), 255);
  EXPECT_EQ(boundary_params.Required<uint8_t>("uint8_min"), 0);

  // Проверяем переполнение (должно возвращать nullopt)
  test_data_["int8_overflow"] = "128";
  test_data_["int8_underflow"] = "-129";
  test_data_["uint8_overflow"] = "256";
  test_data_["uint8_underflow"] = "-1";

  TypedParams overflow_params(test_data_);

  EXPECT_FALSE(overflow_params.TryGet<int8_t>("int8_overflow").has_value());
  EXPECT_FALSE(overflow_params.TryGet<int8_t>("int8_underflow").has_value());
  EXPECT_FALSE(overflow_params.TryGet<uint8_t>("uint8_overflow").has_value());
  EXPECT_FALSE(overflow_params.TryGet<uint8_t>("uint8_underflow").has_value());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
