#pragma once

#include <array>
#include <string_view>

#include "PathParamType.hpp"

static constexpr auto kPathParamTypes = std::to_array<PathParamTypeInfo>(
    {{"string", ".+", PathParamType::String},
     {"int", R"(\d+)", PathParamType::Integer},
     {"float", R"(\d+(\.\d+)?)", PathParamType::Floating}});

constexpr const PathParamTypeInfo* FindPathParamType(const std::string_view name) {
  for (const auto& t : kPathParamTypes) {
    if (t.name == name) {
      return &t;
    }
  }
  return nullptr;
}
