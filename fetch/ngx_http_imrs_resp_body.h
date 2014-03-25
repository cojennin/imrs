#ifndef IMRS_RESP_BODY
#define IMRS_RESP_BODY
#include "ngx_http_imrs_resp_body.h"

extern "C" {
  #include <ngx_config.h>
  #include <ngx_core.h>
  #include <ngx_http.h>
}

class ImrsResponseBody : public ImrsResponse {
    ImrsResponseBody();
};

#endif
