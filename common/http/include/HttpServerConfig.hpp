#pragma once

#include <chrono>

// TODO add config factory or builder pattern
struct HttpServerConfig {
  std::chrono::seconds first_read_timeout{30};
  std::chrono::seconds keep_alive_timeout{15};
  std::size_t header_limit_bytes{8 * 1024};
  std::size_t body_limit_bytes{1024 * 1024};
  std::size_t max_requests_per_connection{50};
  std::size_t max_connections{1000};
};

inline constexpr HttpServerConfig kDefaultHttpServerConfig{};
