#include "ngx_http_imrs_curl.h"

ImrsCurl::ImrsCurl(ngx_str_t u) {
    body = new ImrsCurlBody();
    head = new ImrsCurlHead();
    url = u;
}

ImrsCurl::~ImrsCurl() {
  delete( body );
  delete( head );
}

bool ImrsCurl::downloadImage() {
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

  ngx_memcpy( realUrl, url.data, url.len );

  //buf->data = (char*)malloc(2*1024*1024);
  //buf->size = 0;
  //curl_easy_setopt( curl_h, CURLOPT_VERBOSE, 1 );
  curl_easy_setopt( curl_h, CURLOPT_HEADER, 0 );
  curl_easy_setopt( curl_h, CURLOPT_TIMEOUT, 5 );
  curl_easy_setopt( curl_h, CURLOPT_HEADER, 1 );
  curl_easy_setopt( curl_h, CURLOPT_WRITEFUNCTION, body->writeData );
  curl_easy_setopt( curl_h, CURLOPT_WRITEDATA, body->data );
  curl_easy_setopt( curl_h, CURLOPT_HEADERFUNCTION, head->writeData );
  curl_easy_setopt( curl_h, CURLOPT_WRITEHEADER, head->data );
  curl_easy_setopt( curl_h, CURLOPT_URL, realUrl);
  size_t curl_resp = curl_easy_perform( curl_h );
  curl_global_cleanup();

  return !curl_resp;
}


