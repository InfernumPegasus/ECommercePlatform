#pragma once

#include "JsonHelpers.hpp"

struct User {
  int64_t id;
  std::string status;
  TimestampWithTimezone created_at;
  TimestampWithTimezone updated_at;
};

inline void to_json(nlohmann::json& j, const User& u) {
  j = {{"id", u.id},
       {"status", u.status},
       {"created_at", u.created_at},
       {"updated_at", u.updated_at}};
}

inline void from_json(const nlohmann::json& j, User& u) {
  j.at("id").get_to(u.id);
  j.at("status").get_to(u.status);
  j.at("created_at").get_to(u.created_at);
  j.at("updated_at").get_to(u.updated_at);
}
