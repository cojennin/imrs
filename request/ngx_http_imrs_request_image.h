#ifndef REQUEST_IMAGE_H
#define REQUEST_IMAGE_H
extern "C" {
  #include <ngx_config.h>
  #include <ngx_core.h>
  #include <ngx_http.h>
}

class RequestImage {
  ngx_str_t  source;
  int        width;
  int        height;

  RequestImage() {}
  public:
    RequestImage(ngx_http_request_t*);

    void       setSource(ngx_str_t);
    ngx_str_t  getSource();
    void       setWidth(int);
    size_t     getWidth();
    void       setHeight(int);
    size_t     getHeight();
};

#endif
