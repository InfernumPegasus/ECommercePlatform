#pragma once

#include <nlohmann/json.hpp>
#include <string>

struct OrderCreatedEvent {
  std::string order_id;
  int amount;
};

void to_json(nlohmann::json& j, const OrderCreatedEvent& e);

void from_json(const nlohmann::json& j, OrderCreatedEvent& e);
