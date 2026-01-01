#pragma once

#include "JsonHelpers.hpp"

struct UserCredentials {
  int64_t user_id;
  std::string email;
  std::string password_hash;
  bool email_verified;
  std::optional<TimestampWithTimezone> last_login_at;
};

inline void to_json(nlohmann::json& j, const UserCredentials& c) {
  j = {{"user_id", c.user_id},
       {"email", c.email},
       {"password_hash", c.password_hash},
       {"email_verified", c.email_verified},
       {"last_login_at", c.last_login_at.value_or(nullptr)}};
}

inline void from_json(const nlohmann::json& j, UserCredentials& c) {
  j.at("user_id").get_to(c.user_id);
  j.at("email").get_to(c.email);
  j.at("password_hash").get_to(c.password_hash);
  j.at("email_verified").get_to(c.email_verified);

  if (!j.at("last_login_at").is_null()) {
    j.at("last_login_at").get_to(c.last_login_at);
  }
}
