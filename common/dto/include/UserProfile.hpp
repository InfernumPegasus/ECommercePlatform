#pragma once

#include "JsonHelpers.hpp"

struct UserProfile {
  int64_t user_id;
  std::optional<std::string> display_name;
};

inline void to_json(nlohmann::json& j, const UserProfile& p) {
  j = {{"user_id", p.user_id},
       {"display_name", p.display_name.value_or("")}};
}

inline void from_json(const nlohmann::json& j, UserProfile& p) {
  j.at("user_id").get_to(p.user_id);
  if (j.contains("display_name") && !j.at("display_name").is_null()) {
    p.display_name = j.at("display_name").get<std::string>();
  } else {
    p.display_name.reset();
  }
}
