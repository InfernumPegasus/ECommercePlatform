#pragma once

#include "JsonHelpers.hpp"

struct OrderItem {
  int64_t order_id;
  int64_t product_id;
  int quantity;
  Money price;
};
