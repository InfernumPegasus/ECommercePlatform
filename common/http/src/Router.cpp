#include "Router.hpp"

#include <boost/url.hpp>
#include <iostream>
#include <stdexcept>
#include <string>

#include "HttpErrorMapping.hpp"

namespace {
std::string MethodsMaskToAllowHeader(const RouteTrie::MethodMask& methods_mask) {
  std::string allow_header;
  bool is_first = true;

  for (std::size_t i = 0; i < methods_mask.size(); ++i) {
    if (!methods_mask.test(i)) {
      continue;
    }

    const auto method = static_cast<http::verb>(i);
    const auto method_name = http::to_string(method);
    if (method_name.empty() || method_name == "<unknown>") {
      continue;
    }

    if (!is_first) {
      allow_header += ", ";
    }

    allow_header += method_name;
    is_first = false;
  }

  return allow_header;
}
}  // namespace

Response Router::Route(const Request& req) const {
  const auto parsed = boost::urls::parse_origin_form(req.target());
  if (!parsed) {
    return ErrorResponse(req, HttpError{.status = http::status::bad_request,
                                        .code = "invalid_request_target",
                                        .message = "Invalid request target"});
  }

  std::cout << "Routing request: " << http::to_string(req.method()) << " "
            << parsed->path() << std::endl;

  GeneralParams query;
  for (auto p : parsed->params()) {
    query.emplace(p.key, p.value);
  }

  auto match = trie_.Match(req.method(), parsed->path());

  if (match.IsNotFound()) {
    return ErrorResponse(req, HttpError{.status = http::status::not_found,
                                        .code = "route_not_found",
                                        .message = "Route not found"});
  }

  if (match.IsMethodNotAllowed()) {
    auto res = ErrorResponse(req, HttpError{.status = http::status::method_not_allowed,
                                            .code = "method_not_allowed",
                                            .message = "Method not allowed"});
    res.set(http::field::allow, MethodsMaskToAllowHeader(match.AllowedMethods()));
    return res;
  }

  try {
    auto& matched = match.AsMatched();
    RequestContext ctx(req, std::move(matched.path_params), std::move(query));
    return matched.handler(ctx);
  } catch (const std::exception& ex) {
    std::cerr << "[http] handler error: " << ex.what() << "\n";
    return ErrorResponse(req, MapExceptionToHttpError(ex));
  } catch (...) {
    std::cerr << "[http] handler error: unknown exception\n";
    return ErrorResponse(req, HttpError{.status = http::status::internal_server_error,
                                        .code = "internal_error",
                                        .message = "Internal server error"});
  }
}

void Router::PrintAllRoutes() const {
  const auto routes = trie_.GetAllRoutes();
  std::cout << "All registered routes:" << std::endl;
  for (const auto& route : routes) {
    std::cout << "  " << route << std::endl;
  }
}
