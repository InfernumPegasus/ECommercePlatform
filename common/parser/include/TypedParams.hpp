#pragma once

#include <stdexcept>
#include <string>
#include <unordered_map>
#include <utility>

#include "ValueParser.hpp"

using GeneralParams = std::unordered_map<std::string, std::string>;

class InvalidParameterException final : public std::runtime_error {
 public:
  explicit InvalidParameterException(std::string key)
      : std::runtime_error("Missing or invalid parameter: " + key), key_(std::move(key)) {}

  [[nodiscard]] const std::string& Key() const noexcept { return key_; }

 private:
  std::string key_;
};

class TypedParams {
 public:
  explicit TypedParams(GeneralParams data) : data_(std::move(data)) {}

  template <typename T>
  std::optional<T> TryGet(const std::string& key) const {
    const auto it = data_.find(key);
    if (it == data_.end()) {
      return std::nullopt;
    }
    return value_parser::TryParse<T>(it->second);
  }

  template <typename T>
  T Required(const std::string& key) const {
    const auto value = TryGet<T>(key);
    if (!value) {
      throw InvalidParameterException(key);
    }
    return *value;
  }

  [[nodiscard]] const GeneralParams& GetRawParams() const { return data_; }

 private:
  GeneralParams data_;
};
