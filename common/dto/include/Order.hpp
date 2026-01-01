#pragma once

#include "JsonHelpers.hpp"

struct Order {
  int64_t id;
  int64_t user_id;
  std::string status;
  TimestampWithTimezone created_at;
  TimestampWithTimezone updated_at;
};
