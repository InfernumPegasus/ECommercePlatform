#include "RequestContext.hpp"

RequestContext::RequestContext(const Request& req, const GeneralParams& path_params,
                               const GeneralParams& query_params)
    : request_(req), path_parameters_(path_params), query_parameters_(query_params) {}

const Request& RequestContext::GetRequest() const { return request_; }

const TypedParams& RequestContext::GetPathParameters() const { return path_parameters_; }

const TypedParams& RequestContext::GetQueryParameters() const {
  return query_parameters_;
}
