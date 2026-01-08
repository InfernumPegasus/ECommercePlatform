#pragma once

#include <chrono>
#include <iomanip>
#include <nlohmann/json.hpp>
#include <sstream>
#include <string>

// TODO
using Money = double;
// using Money = boost::multiprecision::cpp_dec_float_50;

// TODO
using TimestampWithTimezone = std::string;

inline std::string to_iso8601(std::chrono::system_clock::time_point tp) {
  std::time_t t = std::chrono::system_clock::to_time_t(tp);
  std::tm tm = *gmtime(&t);

  std::ostringstream oss;
  oss << std::put_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
  return oss.str();
}

inline std::chrono::system_clock::time_point from_iso8601(const std::string& s) {
  std::tm tm{};
  std::istringstream iss(s);
  iss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");

  return std::chrono::system_clock::from_time_t(timegm(&tm));
}
