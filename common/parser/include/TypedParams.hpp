#pragma once

#include <unordered_map>

#include "TypedParams.hpp"
#include "ValueParser.hpp"

using GeneralParams = std::unordered_map<std::string, std::string>;

class TypedParams {
 public:
  explicit TypedParams(const GeneralParams& data) : data_(data) {}

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
      throw std::runtime_error("Missing or invalid parameter: " + key);
    }
    return *value;
  }

  [[nodiscard]] const GeneralParams& GetRawParams() const { return data_; }

 private:
  const GeneralParams& data_;
};
