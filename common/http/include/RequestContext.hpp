#pragma once

#include "HttpTypes.hpp"
#include "TypedParams.hpp"

class RequestContext {
 public:
  RequestContext(const Request& req, GeneralParams&& path_params,
                 GeneralParams&& query_params);

  [[nodiscard]] const Request& GetRequest() const;

  [[nodiscard]] const TypedParams& GetPathParameters() const;

  [[nodiscard]] const TypedParams& GetQueryParameters() const;

 private:
  const Request& request_;
  TypedParams path_parameters_;
  TypedParams query_parameters_;
};