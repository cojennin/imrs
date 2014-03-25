#include "ngx_http_imrs_request_image.h"

RequestImage::RequestImage( ngx_http_request_t* r) {
    height = 0;
    width = 0;
    source = (ngx_str_t)ngx_string('\0');

    if (ngx_http_arg(r, (u_char *) "src", 3, &source) != NGX_OK) {
      throw NGX_ERROR;
    }

    ngx_str_t h = (ngx_str_t)ngx_string('\0');
    ngx_http_arg(r, (u_char*) "h", 1, &h);

    if( h.data == NULL ) {
      height = 100;
    } else {
      height = ngx_atoi( h.data, h.len);
    }

    ngx_str_t w = (ngx_str_t)ngx_string('\0');
    ngx_http_arg(r, (u_char*) "w", 1, &w);

    if( w.data == NULL ) {
      width = 100;
    } else {
      width = ngx_atoi( w.data, w.len );
    }
}

ngx_str_t RequestImage::getSource() {
  return source;
}

size_t RequestImage::getWidth() {
  return width;
}

size_t RequestImage::getHeight() {
  return height;
}
