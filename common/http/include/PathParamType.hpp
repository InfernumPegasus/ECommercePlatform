#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "ValueParser.hpp"

enum class PathParamPriority : uint8_t { Lowest = 0, Low, Medium, High, Highest };

struct PathParamTypeDescriptor {
  std::string_view name;
  PathParamPriority priority;
  bool (*matcher)(std::string_view);
};

constexpr bool MatchInt(const std::string_view value) {
  return value_parser::TryParse<int64_t>(value).has_value();
}

constexpr bool MatchFloat(const std::string_view value) {
  return value_parser::TryParse<double>(value).has_value();
}

constexpr bool MatchString(std::string_view) { return true; }

class PathParamRegistry {
 public:
  static constexpr auto Types = std::to_array<PathParamTypeDescriptor>({
      {
          "string",
          PathParamPriority::Lowest,
          &MatchString,
      },
      {
          "float",
          PathParamPriority::Medium,
          &MatchFloat,
      },
      {
          "int",
          PathParamPriority::Highest,
          &MatchInt,
      },
  });

  static constexpr const PathParamTypeDescriptor* FindByName(
      const std::string_view name) {
    for (const auto& t : Types) {
      if (t.name == name) {
        return &t;
      }
    }
    return nullptr;
  }

  static constexpr const PathParamTypeDescriptor* Default() { return &Types.front(); }
};
