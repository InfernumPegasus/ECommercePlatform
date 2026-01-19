#pragma once

#include <array>
#include <string_view>

#include "ValueParser.hpp"

enum class PathParamType {
  String,
  Integer,
  Floating,
};

enum class PathParamPriority : uint8_t {
  Lowest = 0,
  Low,
  Medium,
  High,
  Highest
};

struct PathParamTypeInfo {
  std::string_view name;
  PathParamType type;
  PathParamPriority priority;
  bool (*matcher)(std::string_view);
};

constexpr bool MatchInt(std::string_view value) {
  return value_parser::TryParse<int64_t>(value).has_value();
}

constexpr bool MatchFloat(std::string_view value) {
  return value_parser::TryParse<double>(value).has_value();
}

constexpr bool MatchString(std::string_view) {
  return true;
}

static constexpr auto kPathParamTypes =
    std::to_array<PathParamTypeInfo>({
        {
            "string",
            PathParamType::String,
            PathParamPriority::Lowest,
            &MatchString,
        },
        {
            "float",
            PathParamType::Floating,
            PathParamPriority::Medium,
            &MatchFloat,
        },
        {
            "int",
            PathParamType::Integer,
            PathParamPriority::Highest,
            &MatchInt,
        },
    });

constexpr const PathParamTypeInfo* FindPathParamType(const std::string_view name) {
  for (const auto& t : kPathParamTypes) {
    if (t.name == name) {
      return &t;
    }
  }
  return nullptr;
}

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
