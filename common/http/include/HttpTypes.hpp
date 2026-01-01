#pragma once

#include <boost/beast/http.hpp>
#include <functional>

namespace http = boost::beast::http;

using Request = http::request<http::string_body>;
using Response = http::response<http::string_body>;
using Handler = std::function<Response(const Request&)>;
