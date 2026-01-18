#pragma once

#include <array>
#include <string_view>

#include "PathParamType.hpp"

static constexpr auto kPathParamTypes =
    std::to_array<PathParamTypeInfo>({{"string", PathParamType::String},
                                      {"int", PathParamType::Integer},
                                      {"float", PathParamType::Floating}}

    );

constexpr bool MatchByType(const PathParamType type, const std::string_view value) {
  switch (type) {
    case PathParamType::Integer:
      return value_parser::TryParse<int64_t>(value).has_value();

    case PathParamType::Floating:
      return value_parser::TryParse<double>(value).has_value();

    case PathParamType::String:
      return true;
  }

  return false;
}

constexpr const PathParamTypeInfo* FindPathParamType(const std::string_view name) {
  for (const auto& t : kPathParamTypes) {
    if (t.name == name) {
      return &t;
    }
  }
  return nullptr;
}
