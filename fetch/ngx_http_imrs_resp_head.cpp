#include "ngx_http_imrs_resp_head.h"

ImrsResponseHead::writeData(void* data, size_t size,
                        size_t nmemb, void* head) {
  head = (data*)head;
  ngx_memcpy( head->data + head->size, data, size*nmemb );
  head->size += size*nmemb;
  return size * nmemb;
}
