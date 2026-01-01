#pragma once

#include "JsonHelpers.hpp"

struct Product {
  int64_t id;
  std::string status;
  TimestampWithTimezone created_at;
  TimestampWithTimezone updated_at;
};
