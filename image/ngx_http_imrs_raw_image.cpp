#include "ngx_http_imrs_raw_image.h"
//Stop syntastic from being annoying

static size_t writeImageToCache(char* img_data, size_t size, size_t nmemb, image* img) {
  ngx_memcpy( img->data + img->size, img_data, size*nmemb );
  img->size += size*nmemb;
  return size * nmemb;
}

RawImage::RawImage(ngx_str_t imgUrl) {
  url = imgUrl;
  buf = (image*)malloc(sizeof(image));
}

RawImage::~RawImage() {
  free( buf );
}

bool RawImage::downloadImage() {
  curl_global_init( CURL_GLOBAL_ALL );
  CURL* curl_h = curl_easy_init();
  char* realUrl;
  realUrl = (char*)malloc(url.len);

  if( !realUrl ) {
    return false;
  }

  //Whelp, something went wrong.
  if( curl_h == NULL ) {
    return false;
  }

  //image* tempBuf = (image*)malloc(sizeof(image));
  //tempBuf->data = (char*)malloc(2*1024*1024);
  //tempBuf->size = 0;
  buf->data = (char*)malloc(2*1024*1024);
  buf->size = 0;
  ngx_memcpy( realUrl, url.data, url.len );
  //curl_easy_setopt( curl_h, CURLOPT_VERBOSE, 1 );
  curl_easy_setopt( curl_h, CURLOPT_HEADER, 0 );
  curl_easy_setopt( curl_h, CURLOPT_TIMEOUT, 5 );
  curl_easy_setopt( curl_h, CURLOPT_WRITEHEADER, 0 );
  curl_easy_setopt( curl_h, CURLOPT_WRITEFUNCTION, writeImageToCache );
  curl_easy_setopt( curl_h, CURLOPT_WRITEDATA, buf );
  curl_easy_setopt( curl_h, CURLOPT_URL, realUrl);
  size_t curl_resp = curl_easy_perform( curl_h );
  curl_global_cleanup();

  return !curl_resp;
}

image* RawImage::getImage() {
  return buf;
}

char* RawImage::getData() {
  return buf->data;
}

size_t RawImage::getSize() {
  return buf->size;
}
