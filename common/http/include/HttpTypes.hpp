#pragma once

#include <boost/beast/http.hpp>

namespace http = boost::beast::http;

using Request = http::request<http::string_body>;
using Response = http::response<http::string_body>;
