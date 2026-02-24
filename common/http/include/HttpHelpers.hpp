#pragma once

#include <nlohmann/json.hpp>

#include "HttpError.hpp"
#include "HttpTypes.hpp"

Response JsonResponse(const Request& req, http::status status,
                      const nlohmann::json& body);

Response ErrorResponse(const Request& req, const HttpError& error);
