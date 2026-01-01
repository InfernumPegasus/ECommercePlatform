#pragma once

#include "JsonHelpers.hpp"

struct ProductAttribute {
  int64_t product_id;
  std::string title;
  std::optional<std::string> description;
};
