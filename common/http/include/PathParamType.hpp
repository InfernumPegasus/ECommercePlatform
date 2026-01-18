#pragma once

#include <string_view>

enum class PathParamType {
  String,
  Integer,
  Floating,
};

struct PathParamTypeInfo {
  std::string_view name;
  PathParamType type;
};
