#include "ngx_http_imrs_resp_body.h"

ImrsResponseBody::writeData(void* data, size_t size,
                        size_t nmemb, void* body) {
  body = (data*)head;
  ngx_memcpy( body->data + body->size, data, size*nmemb );
  body->size += size*nmemb;
  return size * nmemb;
}
