#pragma once

#include <charconv>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>

using GeneralParams = std::unordered_map<std::string, std::string>;

// Concepts

template <typename T>
concept Numeric =
    (std::is_integral_v<T> && !std::is_same_v<T, bool>) || std::is_floating_point_v<T>;

template <typename T>
concept Bool = std::is_same_v<T, bool>;

template <typename T>
concept StringLike =
    std::is_same_v<T, std::string> || std::is_same_v<T, std::string_view>;

class TypedParams {
 public:
  explicit TypedParams(const GeneralParams& data) : data_(data) {}

  // ---------- parse<T> ----------

  template <Numeric T>
  static std::optional<T> parse(std::string_view value) {
    T result{};

    auto [ptr, ec] = std::from_chars(value.data(), value.data() + value.size(), result);

    if (ec != std::errc{} || ptr != value.data() + value.size()) {
      return std::nullopt;
    }

    return result;
  }

  template <Bool T>
  static std::optional<bool> parse(const std::string_view value) {
    std::string lower_value;
    lower_value.reserve(value.size());
    std::transform(value.begin(), value.end(), std::back_inserter(lower_value),
                   [](const unsigned char c) { return std::tolower(c); });

    if (lower_value == "true" || lower_value == "1") {
      return true;
    }
    if (lower_value == "false" || lower_value == "0") {
      return false;
    }
    return std::nullopt;
  }

  template <StringLike T>
  static std::optional<T> parse(std::string_view value) {
    if constexpr (std::is_same_v<T, std::string>) {
      return std::string(value);
    } else {
      return value;
    }
  }

  // ---------- accessors ----------

  template <typename T>
  std::optional<T> get(const std::string& key) const {
    const auto it = data_.find(key);
    if (it == data_.end()) {
      return std::nullopt;
    }
    return parse<T>(it->second);
  }

  template <typename T>
  T required(const std::string& key) const {
    const auto value = get<T>(key);
    if (!value) {
      throw std::runtime_error("Missing or invalid parameter: " + key);
    }
    return *value;
  }

 private:
  const GeneralParams& data_;
};