#pragma once

#include <algorithm>
#include <cctype>
#include <charconv>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

// Concepts

template <typename T>
concept Numeric =
    (std::is_integral_v<T> && !std::is_same_v<T, bool>) || std::is_floating_point_v<T>;

template <typename T>
concept Bool = std::is_same_v<T, bool>;

template <typename T>
concept StringLike =
    std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>;

// String to value parsers
namespace value_parser {

namespace detail {
inline constexpr std::string StringToLower(std::string_view value) {
  std::string lower_value;
  lower_value.reserve(value.size());
  std::ranges::transform(value, std::back_inserter(lower_value),
                         [](const unsigned char c) { return std::tolower(c); });

  return lower_value;
}
}  // namespace detail

template <Numeric T>
std::optional<T> TryParse(std::string_view value) {
  T result{};

  if constexpr (std::is_floating_point_v<T>) {
    static constexpr size_t NAN_INF_LITERAL_LENGTH = 3;

    if (value.size() == NAN_INF_LITERAL_LENGTH) {
      const std::string lower_value = detail::StringToLower(value);

      if (lower_value == "inf" || lower_value == "nan") {
        return std::nullopt;
      }
    }
  }

  auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), result);

  if (ec != std::errc{} || ptr != value.data() + value.size()) {
    return std::nullopt;
  }

  return result;
}

template <Bool T>
std::optional<bool> TryParse(const std::string_view value) {
  static constexpr size_t FALSE_LITERAL_LENGTH = 5;
  if (value.empty() || value.size() > FALSE_LITERAL_LENGTH) {
    return std::nullopt;
  }

  const std::string lower_value = detail::StringToLower(value);

  if (lower_value == "true" || lower_value == "1") {
    return true;
  }
  if (lower_value == "false" || lower_value == "0") {
    return false;
  }
  return std::nullopt;
}

template <StringLike T>
std::optional<T> TryParse(std::string_view value) {
  if constexpr (std::is_same_v<T, std::string>) {
    return std::string(value);
  } else {
    return value;
  }
}

}  // namespace value_parser
