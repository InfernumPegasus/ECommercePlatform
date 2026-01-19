#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "ValueParser.hpp"

enum class PathParamType {
  String,
  Integer,
  Floating,
};

enum class PathParamPriority : uint8_t { Lowest = 0, Low, Medium, High, Highest };

struct PathParamTypeInfo {
  PathParamType type;
  std::string_view name;
  PathParamPriority priority;
  bool (*matcher)(std::string_view);
};

constexpr bool MatchInt(const std::string_view value) {
  return value_parser::TryParse<std::int64_t>(value).has_value();
}

constexpr bool MatchFloat(const std::string_view value) {
  return value_parser::TryParse<std::double_t>(value).has_value();
}

constexpr bool MatchString(std::string_view) { return true; }

class PathParamRegistry {
 public:
  static constexpr auto Types = std::to_array<PathParamTypeInfo>({
      {
          PathParamType::String,
          "string",
          PathParamPriority::Lowest,
          &MatchString,
      },
      {
          PathParamType::Floating,
          "float",
          PathParamPriority::Medium,
          &MatchFloat,
      },
      {
          PathParamType::Integer,
          "int",
          PathParamPriority::Highest,
          &MatchInt,
      },
  });

  static constexpr const PathParamTypeInfo* FindByName(const std::string_view name) {
    for (const auto& t : Types) {
      if (t.name == name) {
        return &t;
      }
    }
    return nullptr;
  }

  static constexpr const PathParamTypeInfo* FindByType(const PathParamType type) {
    for (const auto& t : Types) {
      if (t.type == type) {
        return &t;
      }
    }
    return nullptr;
  }

  static constexpr const PathParamTypeInfo* Default() {
    return FindByType(PathParamType::String);
  }
};
