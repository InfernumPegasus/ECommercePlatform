#pragma once

#include <nlohmann/json.hpp>

#include "HttpTypes.hpp"

Response json_response(const Request& req, http::status status,
                       const nlohmann::json& body);
