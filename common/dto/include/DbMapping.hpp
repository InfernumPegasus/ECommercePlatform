#pragma once

#include <pqxx/pqxx>
#include <tuple>
#include <utility>

template <typename T>
struct DbMapping;

template <typename T>
constexpr T MapRowTo(const pqxx::row& row) {
  static_assert(std::is_default_constructible_v<T>);

  T result{};

  std::apply(
      [&](auto... field) {
        (..., ([&] {
           const auto& [column, member] = field;

           using FieldType = std::remove_reference_t<decltype(result.*member)>;

           result.*member = row[column].template as<FieldType>();
         }()));
      },
      DbMapping<T>::fields);

  return result;
}