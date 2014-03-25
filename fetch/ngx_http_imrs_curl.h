#ifndef IMRS_CURL_H
#define IMRS_CURL_H

extern "C" {
  #include <ngx_config.h>
  #include <ngx_core.h>
}

#include "ngx_http_imrs_resp_head.h"
#include "ngx_http_imrs_resp_body.h"

class ImrsCurl {
    ngx_str_t url;
    ImrsResponseBody* body;
    ImrsResponseHead* head;

  public:
    ImrsCurl(ngx_str_t);
    ~ImrsCurl();

    bool downloadImage();

    void setUrl(ngx_str_t);
    void setUrl(char*);
    ngx_str_t getUrl();
};

#endif
