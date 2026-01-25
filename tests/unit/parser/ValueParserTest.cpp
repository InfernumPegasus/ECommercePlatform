#include <gtest/gtest.h>

#include <string>
#include <string_view>

#include "ValueParser.hpp"

using namespace value_parser;

class ValueParserTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

// Тесты для числовых типов (целые числа)
TEST_F(ValueParserTest, ParseValidIntegers) {
  // int32_t
  EXPECT_EQ(TryParse<int32_t>("42"), 42);
  EXPECT_EQ(TryParse<int32_t>("-42"), -42);
  EXPECT_EQ(TryParse<int32_t>("0"), 0);
  EXPECT_EQ(TryParse<int32_t>("2147483647"), 2147483647);
  EXPECT_EQ(TryParse<int32_t>("-2147483648"), -2147483648);

  // int64_t
  EXPECT_EQ(TryParse<int64_t>("9223372036854775807"), 9223372036854775807LL);
  EXPECT_EQ(TryParse<int64_t>("-9223372036854775808"), -9223372036854775808LL);

  // uint32_t
  EXPECT_EQ(TryParse<uint32_t>("4294967295"), 4294967295U);
  EXPECT_EQ(TryParse<uint32_t>("0"), 0U);

  // uint64_t
  EXPECT_EQ(TryParse<uint64_t>("18446744073709551615"), 18446744073709551615ULL);
}

TEST_F(ValueParserTest, ParseInvalidIntegers) {
  // Пустая строка
  EXPECT_FALSE(TryParse<int>("").has_value());

  // Не числа
  EXPECT_FALSE(TryParse<int>("abc").has_value());
  EXPECT_FALSE(TryParse<int>("123abc").has_value());
  EXPECT_FALSE(TryParse<int>("abc123").has_value());
  EXPECT_FALSE(TryParse<int>("12.3").has_value());  // float вместо int

  // С пробелами
  EXPECT_FALSE(TryParse<int>(" 123").has_value());
  EXPECT_FALSE(TryParse<int>("123 ").has_value());
  EXPECT_FALSE(TryParse<int>(" 123 ").has_value());

  // Переполнение
  EXPECT_FALSE(TryParse<int32_t>("9999999999").has_value());
  EXPECT_FALSE(TryParse<uint32_t>("-1").has_value());
  EXPECT_FALSE(TryParse<uint64_t>("-1").has_value());
}

// Тесты для чисел с плавающей точкой
TEST_F(ValueParserTest, ParseValidFloats) {
  // double
  EXPECT_DOUBLE_EQ(*TryParse<double>("3.14"), 3.14);
  EXPECT_DOUBLE_EQ(*TryParse<double>("-3.14"), -3.14);
  EXPECT_DOUBLE_EQ(*TryParse<double>("0.0"), 0.0);
  EXPECT_DOUBLE_EQ(*TryParse<double>("1.0"), 1.0);
  EXPECT_DOUBLE_EQ(*TryParse<double>(".5"), 0.5);
  EXPECT_DOUBLE_EQ(*TryParse<double>("5."), 5.0);

  // Научная нотация
  EXPECT_DOUBLE_EQ(*TryParse<double>("1.23e4"), 12300.0);
  EXPECT_DOUBLE_EQ(*TryParse<double>("-1.23e-4"), -0.000123);

  // float
  EXPECT_FLOAT_EQ(*TryParse<float>("2.718"), 2.718f);
}

TEST_F(ValueParserTest, ParseInvalidFloats) {
  // Невалидные float
  EXPECT_FALSE(TryParse<double>("").has_value());
  EXPECT_FALSE(TryParse<double>("abc").has_value());
  EXPECT_FALSE(TryParse<double>("1.2.3").has_value());
  EXPECT_FALSE(TryParse<double>("1,2").has_value());  // запятая вместо точки
  EXPECT_FALSE(TryParse<double>("inf").has_value());
  EXPECT_FALSE(TryParse<double>("NaN").has_value());

  // С пробелами
  EXPECT_FALSE(TryParse<double>(" 3.14").has_value());
  EXPECT_FALSE(TryParse<double>("3.14 ").has_value());
}

// Тесты для булевых значений
TEST_F(ValueParserTest, ParseValidBooleans) {
  // true значения
  EXPECT_EQ(*TryParse<bool>("true"), true);
  EXPECT_EQ(*TryParse<bool>("TRUE"), true);
  EXPECT_EQ(*TryParse<bool>("True"), true);
  EXPECT_EQ(*TryParse<bool>("1"), true);

  // false значения
  EXPECT_EQ(*TryParse<bool>("false"), false);
  EXPECT_EQ(*TryParse<bool>("FALSE"), false);
  EXPECT_EQ(*TryParse<bool>("False"), false);
  EXPECT_EQ(*TryParse<bool>("0"), false);
}

TEST_F(ValueParserTest, ParseInvalidBooleans) {
  // Невалидные булевы значения
  EXPECT_FALSE(TryParse<bool>("").has_value());
  EXPECT_FALSE(TryParse<bool>("yes").has_value());
  EXPECT_FALSE(TryParse<bool>("no").has_value());
  EXPECT_FALSE(TryParse<bool>("t").has_value());
  EXPECT_FALSE(TryParse<bool>("f").has_value());
  EXPECT_FALSE(TryParse<bool>("2").has_value());
  EXPECT_FALSE(TryParse<bool>("-1").has_value());
  EXPECT_FALSE(TryParse<bool>("true ").has_value());
  EXPECT_FALSE(TryParse<bool>(" true").has_value());
}

// Тесты для строковых типов
TEST_F(ValueParserTest, ParseValidStrings) {
  // std::string
  EXPECT_EQ(*TryParse<std::string>("hello"), "hello");
  EXPECT_EQ(*TryParse<std::string>(""), "");
  EXPECT_EQ(*TryParse<std::string>("123"), "123");
  EXPECT_EQ(*TryParse<std::string>("hello world"), "hello world");
  EXPECT_EQ(*TryParse<std::string>("привет"), "привет");  // Unicode
  EXPECT_EQ(*TryParse<std::string>("special!@#$%"), "special!@#$%");

  // std::string_view
  std::string_view result = *TryParse<std::string_view>("test");
  EXPECT_EQ(result, "test");
}

TEST_F(ValueParserTest, ParseStringViewReturnsView) {
  std::string original = "original string";
  auto result = TryParse<std::string_view>(original);

  EXPECT_TRUE(result.has_value());
  EXPECT_EQ(*result, original);

  // Проверяем, что это действительно view (указывает на ту же память)
  EXPECT_EQ(result->data(), original.data());
}

// Тесты для граничных случаев
TEST_F(ValueParserTest, EdgeCases) {
  // Очень большие числа (но валидные)
  EXPECT_TRUE(TryParse<int64_t>("999999999999999999").has_value());

  // Ноль в разных представлениях
  EXPECT_EQ(*TryParse<int>("0"), 0);
  EXPECT_EQ(*TryParse<double>("0.0"), 0.0);
  EXPECT_EQ(*TryParse<bool>("0"), false);
  EXPECT_EQ(*TryParse<std::string>("0"), "0");

  // Минус ноль
  EXPECT_EQ(*TryParse<int>("-0"), 0);
  EXPECT_EQ(*TryParse<double>("-0.0"), -0.0);
}

// Тесты для разных целочисленных типов
TEST_F(ValueParserTest, DifferentIntegerTypes) {
  // int8_t
  EXPECT_EQ(*TryParse<int8_t>("127"), 127);
  EXPECT_EQ(*TryParse<int8_t>("-128"), -128);
  EXPECT_FALSE(TryParse<int8_t>("128").has_value());   // переполнение
  EXPECT_FALSE(TryParse<int8_t>("-129").has_value());  // underflow

  // int16_t
  EXPECT_EQ(*TryParse<int16_t>("32767"), 32767);
  EXPECT_EQ(*TryParse<int16_t>("-32768"), -32768);

  // uint8_t
  EXPECT_EQ(*TryParse<uint8_t>("255"), 255);
  EXPECT_EQ(*TryParse<uint8_t>("0"), 0);
  EXPECT_FALSE(TryParse<uint8_t>("256").has_value());
  EXPECT_FALSE(TryParse<uint8_t>("-1").has_value());

  // uint16_t
  EXPECT_EQ(*TryParse<uint16_t>("65535"), 65535);
}

// Тест на корректность возвращаемого типа optional
TEST_F(ValueParserTest, OptionalBehavior) {
  // Валидный парсинг возвращает optional с значением
  auto valid_int = TryParse<int>("42");
  EXPECT_TRUE(valid_int.has_value());
  EXPECT_EQ(*valid_int, 42);

  // Невалидный парсинг возвращает empty optional
  auto invalid_int = TryParse<int>("abc");
  EXPECT_FALSE(invalid_int.has_value());

  // Можно использовать value_or
  EXPECT_EQ(TryParse<int>("123").value_or(-1), 123);
  EXPECT_EQ(TryParse<int>("abc").value_or(-1), -1);
}

// Тесты для производительности/корректности
TEST_F(ValueParserTest, PerformanceAndCorrectness) {
  // Многократный парсинг одного значения
  for (int i = 0; i < 100; ++i) {
    EXPECT_EQ(*TryParse<int>("42"), 42);
  }

  // Разные типы одного значения
  EXPECT_EQ(*TryParse<int>("100"), 100);
  EXPECT_EQ(*TryParse<double>("100"), 100.0);
  EXPECT_EQ(*TryParse<std::string>("100"), "100");
  EXPECT_EQ(*TryParse<bool>("1"), true);
}

// Тесты для специальных строковых случаев
TEST_F(ValueParserTest, SpecialStringCases) {
  // Строки, которые выглядят как числа
  EXPECT_EQ(*TryParse<std::string>("123"), "123");
  EXPECT_EQ(*TryParse<std::string>("3.14"), "3.14");
  EXPECT_EQ(*TryParse<std::string>("true"), "true");
  EXPECT_EQ(*TryParse<std::string>("false"), "false");

  // Строки с пробелами
  EXPECT_EQ(*TryParse<std::string>("hello world"), "hello world");
  EXPECT_EQ(*TryParse<std::string>("  leading"), "  leading");
  EXPECT_EQ(*TryParse<std::string>("trailing  "), "trailing  ");
  EXPECT_EQ(*TryParse<std::string>("  both  "), "  both  ");

  // Строки с управляющими символами
  EXPECT_EQ(*TryParse<std::string>("\n\t\r"), "\n\t\r");

  // Длинные строки
  std::string long_string(1000, 'a');
  EXPECT_EQ(*TryParse<std::string>(long_string), long_string);
}

// Тесты для концептов (compile-time проверки)
TEST_F(ValueParserTest, ConceptValidation) {
  // Проверяем, что концепты работают корректно
  static_assert(Numeric<int>);
  static_assert(Numeric<double>);
  static_assert(Numeric<int64_t>);
  static_assert(!Numeric<bool>);  // bool не считается Numeric
  static_assert(!Numeric<std::string>);

  static_assert(Bool<bool>);
  static_assert(!Bool<int>);
  static_assert(!Bool<std::string>);

  static_assert(StringLike<std::string>);
  static_assert(StringLike<std::string_view>);
  static_assert(!StringLike<int>);
  static_assert(!StringLike<const char*>);  // Не поддерживается
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
