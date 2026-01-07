#pragma once

#include "HttpTypes.hpp"
#include "TypedParams.hpp"

class RequestContext {
 public:
  RequestContext(const Request& req, const GeneralParams& path_params,
                 const GeneralParams& query_params);

  [[nodiscard]] const Request& GetRequest() const;

  [[nodiscard]] const TypedParams& GetPathParameters() const;

  [[nodiscard]] const TypedParams& GetQueryParameters() const;

 private:
  const Request& request_;
  TypedParams path_parameters_;
  TypedParams query_parameters_;
};