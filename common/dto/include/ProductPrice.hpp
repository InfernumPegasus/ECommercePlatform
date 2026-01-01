#pragma once

#include "JsonHelpers.hpp"

struct ProductPrice {
  int64_t id;
  int64_t product_id;
  Money price;
  TimestampWithTimezone valid_from;
  std::optional<TimestampWithTimezone> valid_to;
};

inline void to_json(nlohmann::json& j, const ProductPrice& p) {
  j = {{"id", p.id},
       {"product_id", p.product_id},
       {"price", p.price},
       {"valid_from", p.valid_from},
       {"valid_to", p.valid_to ? p.valid_to.value() : nullptr}};
}

inline void from_json(const nlohmann::json& j, ProductPrice& p) {
  j.at("id").get_to(p.id);
  j.at("product_id").get_to(p.product_id);
  j.at("price").get_to(p.price);
  j.at("valid_from").get_to(p.valid_from);

  if (!j.at("valid_to").is_null()) {
    j.at("valid_to").get_to(p.valid_to);
  }
}
