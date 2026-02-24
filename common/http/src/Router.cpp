#include "Router.hpp"

#include <boost/url.hpp>
#include <iostream>
#include <stdexcept>

#include "HttpErrorMapping.hpp"

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

  auto [handler, path_params] = trie_.FindRoute(req.method(), parsed->path());
  if (!handler) {
    return ErrorResponse(req, HttpError{.status = http::status::not_found,
                                        .code = "route_not_found",
                                        .message = "Route not found"});
  }

  try {
    RequestContext ctx(req, std::move(path_params), std::move(query));
    return handler(ctx);
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
