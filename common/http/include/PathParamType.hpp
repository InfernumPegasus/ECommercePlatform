#pragma once

#include <array>
#include <string_view>

#include "ValueParser.hpp"

enum class PathParamType {
  String,
  Integer,
  Floating,
};

struct PathParamTypeInfo {
  std::string_view name;
  PathParamType type;
};

constexpr std::string_view ToString(const PathParamType type) {
  switch (type) {
    case PathParamType::String:
      return "string";
    case PathParamType::Integer:
      return "int";
    case PathParamType::Floating:
      return "float";
  }
  return "unknown";
}

/*
 * Priorities defined according to sets theory. Each type is represented as set:
 * - String is the most common set (every value is a string by default), so put it down
 * - Float is kind of String which must be represented only using digits, plus or minus
 * sign, and single delimiter
 * - Integer is kind of String which must be represented only using digits and plus or
 * minus sign. These criteria make Integer the most specific of these types, so priority
 * is maxed
 *
 * In short: Integer set is more specific than Float set, Float set is more specific than
 * String set
 */
constexpr int Priority(const PathParamType type) {
  switch (type) {
    case PathParamType::Integer:
      return 3;
    case PathParamType::Floating:
      return 2;
    case PathParamType::String:
      return 1;
  }
  return 0;
}

constexpr bool Match(const PathParamType type, const std::string_view value) {
  switch (type) {
    case PathParamType::Integer:
      return value_parser::TryParse<int64_t>(value).has_value();

    case PathParamType::Floating:
      return value_parser::TryParse<std::double_t>(value).has_value();

    case PathParamType::String:
      return true;
  }
  return false;
}

static constexpr auto kPathParamTypes =
    std::to_array<PathParamTypeInfo>({{"string", PathParamType::String},
                                      {"int", PathParamType::Integer},
                                      {"float", PathParamType::Floating}});

constexpr const PathParamTypeInfo* FindPathParamType(const std::string_view name) {
  for (const auto& t : kPathParamTypes) {
    if (t.name == name) {
      return &t;
    }
  }
  return nullptr;
}
