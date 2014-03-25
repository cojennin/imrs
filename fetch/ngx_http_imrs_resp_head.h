#ifndef IMRS_RESP_HEAD
#define IMRS_RESP_HEAD
#include "ngx_http_imrs_response.h"

extern "C" {
  #include <ngx_config.h>
  #include <ngx_core.h>
  #include <ngx_http.h>
}

class ImrsResponseHead : public ImrsResponse {
    ImrsResponseHead();
};

#endif
