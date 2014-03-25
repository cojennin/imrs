#ifndef RAW_IMAGE_H
#define RAW_IMAGE_H

extern "C" {
  #include <ngx_config.h>
  #include <ngx_core.h>
  #include <curl/curl.h>
}

typedef struct {
  char* data;
  size_t size;
} image;

class RawImage {
  ngx_str_t   url;
  image* buf;

  RawImage() {};
  public:
    RawImage(ngx_str_t);
    ~RawImage();

    bool downloadImage();
    void setUrl(ngx_str_t);
    ngx_str_t getUrl();
    size_t getSize();
    char* getData();
    image* getImage();
};

#endif
