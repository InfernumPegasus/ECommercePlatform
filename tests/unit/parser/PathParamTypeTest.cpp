#include <gtest/gtest.h>

#include <string_view>

#include "PathParamType.hpp"

class PathParamTypeTest : public ::testing::Test {
 protected:
  void SetUp() override {}
  void TearDown() override {}
};

// Тесты для MatchInt
TEST_F(PathParamTypeTest, MatchIntValid) {
  EXPECT_TRUE(MatchInt("0"));
  EXPECT_TRUE(MatchInt("123"));
  EXPECT_TRUE(MatchInt("-456"));
  EXPECT_FALSE(MatchInt("+789"));
  EXPECT_TRUE(MatchInt("2147483647"));
  EXPECT_TRUE(MatchInt("-2147483648"));
  EXPECT_TRUE(MatchInt("999999999999999"));
}

TEST_F(PathParamTypeTest, MatchIntInvalid) {
  EXPECT_FALSE(MatchInt(""));
  EXPECT_FALSE(MatchInt("abc"));
  EXPECT_FALSE(MatchInt("123abc"));
  EXPECT_FALSE(MatchInt("12.3"));
  EXPECT_FALSE(MatchInt("12,3"));
  EXPECT_FALSE(MatchInt(" 123"));
  EXPECT_FALSE(MatchInt("123 "));
  EXPECT_FALSE(MatchInt("12 34"));
  EXPECT_FALSE(MatchInt("0x1A"));  // hex не поддерживается
  EXPECT_TRUE(MatchInt("0123"));   // leading zero допустим
}

// Тесты для MatchFloat
TEST_F(PathParamTypeTest, MatchFloatValid) {
  EXPECT_TRUE(MatchFloat("0.0"));
  EXPECT_TRUE(MatchFloat("3.14"));
  EXPECT_TRUE(MatchFloat("-2.718"));
  EXPECT_TRUE(MatchFloat(".5"));
  EXPECT_TRUE(MatchFloat("5."));
  EXPECT_TRUE(MatchFloat("1.23e4"));
  EXPECT_TRUE(MatchFloat("-1.23e-4"));
  EXPECT_TRUE(MatchFloat("0"));
  EXPECT_TRUE(MatchFloat("123"));  // целые числа тоже float
  EXPECT_TRUE(MatchFloat("-456"));
}

TEST_F(PathParamTypeTest, MatchFloatInvalid) {
  EXPECT_FALSE(MatchFloat(""));
  EXPECT_FALSE(MatchFloat("abc"));
  EXPECT_FALSE(MatchFloat("1.2.3"));
  EXPECT_FALSE(MatchFloat("1,2"));
  EXPECT_FALSE(MatchFloat(" 3.14"));
  EXPECT_FALSE(MatchFloat("3.14 "));
  EXPECT_FALSE(MatchFloat("inf"));
  EXPECT_FALSE(MatchFloat("NaN"));
  EXPECT_FALSE(MatchFloat("1.2e"));
  EXPECT_FALSE(MatchFloat("e10"));
}

// Тесты для MatchString (всегда true)
TEST_F(PathParamTypeTest, MatchStringAlwaysTrue) {
  EXPECT_TRUE(MatchString(""));
  EXPECT_TRUE(MatchString("abc"));
  EXPECT_TRUE(MatchString("123"));
  EXPECT_TRUE(MatchString("12.3"));
  EXPECT_TRUE(MatchString("true"));
  EXPECT_TRUE(MatchString("hello world"));
  EXPECT_TRUE(MatchString("special!@#$%"));
  EXPECT_TRUE(MatchString("привет"));  // unicode
  EXPECT_TRUE(MatchString("  spaced  "));
}

// Тесты для PathParamRegistry
TEST_F(PathParamTypeTest, PathParamRegistryFindByName) {
  // Существующие типы
  EXPECT_NE(PathParamRegistry::FindByName("int"), nullptr);
  EXPECT_NE(PathParamRegistry::FindByName("float"), nullptr);
  EXPECT_NE(PathParamRegistry::FindByName("string"), nullptr);

  // Проверяем корректность данных
  auto* int_type = PathParamRegistry::FindByName("int");
  EXPECT_EQ(int_type->name, "int");
  EXPECT_EQ(int_type->priority, PathParamPriority::Highest);
  EXPECT_NE(int_type->matcher, nullptr);

  auto* float_type = PathParamRegistry::FindByName("float");
  EXPECT_EQ(float_type->name, "float");
  EXPECT_EQ(float_type->priority, PathParamPriority::Medium);
  EXPECT_NE(float_type->matcher, nullptr);

  auto* string_type = PathParamRegistry::FindByName("string");
  EXPECT_EQ(string_type->name, "string");
  EXPECT_EQ(string_type->priority, PathParamPriority::Lowest);
  EXPECT_NE(string_type->matcher, nullptr);

  // Несуществующие типы
  EXPECT_EQ(PathParamRegistry::FindByName("nonexistent"), nullptr);
  EXPECT_EQ(PathParamRegistry::FindByName(""), nullptr);
  EXPECT_EQ(PathParamRegistry::FindByName("INT"), nullptr);  // case sensitive
  EXPECT_EQ(PathParamRegistry::FindByName("Int"), nullptr);
}

TEST_F(PathParamTypeTest, PathParamRegistryDefault) {
  auto* default_type = PathParamRegistry::Default();
  EXPECT_NE(default_type, nullptr);

  // По умолчанию должен быть string
  EXPECT_EQ(default_type->name, "string");
  EXPECT_EQ(default_type->priority, PathParamPriority::Lowest);
}

TEST_F(PathParamTypeTest, PathParamPriorityOrdering) {
  // Проверяем порядок приоритетов
  EXPECT_LT(static_cast<uint8_t>(PathParamPriority::Lowest),
            static_cast<uint8_t>(PathParamPriority::Low));
  EXPECT_LT(static_cast<uint8_t>(PathParamPriority::Low),
            static_cast<uint8_t>(PathParamPriority::Medium));
  EXPECT_LT(static_cast<uint8_t>(PathParamPriority::Medium),
            static_cast<uint8_t>(PathParamPriority::High));
  EXPECT_LT(static_cast<uint8_t>(PathParamPriority::High),
            static_cast<uint8_t>(PathParamPriority::Highest));
}

// ParamKey is an internal detail of RouteTrie and is intentionally not tested here.

// Интеграционные тесты для всех типов параметров
TEST_F(PathParamTypeTest, IntegrationAllTypes) {
  // Тестируем все зарегистрированные типы
  for (const auto& type : PathParamRegistry::Types) {
    EXPECT_FALSE(type.name.empty());
    EXPECT_NE(type.matcher, nullptr);

    // Проверяем, что матчеры работают
    if (type.name == "int") {
      EXPECT_TRUE(type.matcher("123"));
      EXPECT_FALSE(type.matcher("abc"));
      EXPECT_FALSE(type.matcher("12.3"));
    } else if (type.name == "float") {
      EXPECT_TRUE(type.matcher("123"));
      EXPECT_TRUE(type.matcher("12.3"));
      EXPECT_FALSE(type.matcher("abc"));
    } else if (type.name == "string") {
      EXPECT_TRUE(type.matcher("anything"));
      EXPECT_TRUE(type.matcher(""));
      EXPECT_TRUE(type.matcher("123"));
      EXPECT_TRUE(type.matcher("12.3"));
    }
  }
}

// Тесты для крайних случаев
TEST_F(PathParamTypeTest, EdgeCases) {
  // Очень длинные строки
  std::string long_string(10000, 'a');
  EXPECT_TRUE(MatchString(long_string));

  // Строки с нулевым байтом
  std::string with_null = "test\0test";
  EXPECT_TRUE(MatchString(std::string_view(with_null.data(), with_null.size() + 5)));

  // Граничные значения чисел
  EXPECT_TRUE(MatchInt("9223372036854775807"));   // int64 max
  EXPECT_TRUE(MatchInt("-9223372036854775808"));  // int64 min

  // Очень большие/маленькие float
  EXPECT_TRUE(MatchFloat("1.7e308"));   // около double max
  EXPECT_TRUE(MatchFloat("-1.7e308"));  // около double min
  EXPECT_TRUE(MatchFloat("1e-308"));    // около double min positive
}

// Тесты для приоритетов выбора типа
TEST_F(PathParamTypeTest, TypeSelectionPriority) {
  // Симуляция выбора типа для значения "123"
  // Это может быть и int, и float, и string
  // Но int должен иметь высший приоритет

  bool matched_int = false;
  bool matched_float = false;
  bool matched_string = false;

  for (const auto& type : PathParamRegistry::Types) {
    if (type.matcher("123")) {
      if (type.name == "int") matched_int = true;
      if (type.name == "float") matched_float = true;
      if (type.name == "string") matched_string = true;
    }
  }

  // "123" должно матчиться на все три типа
  EXPECT_TRUE(matched_int);
  EXPECT_TRUE(matched_float);
  EXPECT_TRUE(matched_string);

  // Но int должен иметь высший приоритет
  auto* int_type = PathParamRegistry::FindByName("int");
  auto* float_type = PathParamRegistry::FindByName("float");
  auto* string_type = PathParamRegistry::FindByName("string");

  EXPECT_TRUE(int_type->priority > float_type->priority);
  EXPECT_TRUE(int_type->priority > string_type->priority);
  EXPECT_TRUE(float_type->priority > string_type->priority);
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
