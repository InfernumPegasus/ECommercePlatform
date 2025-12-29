#include "OrderCreatedEvent.hpp"

void to_json(nlohmann::json& j, const OrderCreatedEvent& e) {
  j["order_id"] = e.order_id;
  j["amount"] = e.amount;
}

void from_json(const nlohmann::json& j, OrderCreatedEvent& e) {
  j.at("order_id").get_to(e.order_id);
  j.at("amount").get_to(e.amount);
}
