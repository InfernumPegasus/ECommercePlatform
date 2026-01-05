#pragma once

#include <boost/beast/http.hpp>

#include "TypedParams.hpp"

namespace http = boost::beast::http;

using Request = http::request<http::string_body>;
using Response = http::response<http::string_body>;

struct RequestContext {
  const Request& request;
  TypedParams path;
  TypedParams query;

  RequestContext(const Request& req, const GeneralParams& path_params,
                 const GeneralParams& query_params)
      : request(req), path(path_params), query(query_params) {}
};
