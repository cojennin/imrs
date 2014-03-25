#ifndef NGX_HTTP_IMRS_H
#define NGX_HTTP_IMRS_H

extern "C" {
  #include <ngx_config.h>
  #include <ngx_core.h>
  #include <ngx_http.h>
  #include <ngx_md5.h>

  ngx_str_t image_types[3] = {
    ngx_string("image/jpeg"),
    ngx_string("image/gif"),
    ngx_string("image/png")
  };
  typedef struct {} ngx_http_imrs_loc_conf_t;

  static char* ngx_http_imrs(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
  static void* ngx_http_imrs_create_loc_conf(ngx_conf_t *cf);
  static char* ngx_http_imrs_merge_loc_conf(ngx_conf_t *cf,
      void *parent, void *child);
  ngx_int_t ngx_http_imrs_init_module(ngx_cycle_t *cycle);
  ngx_int_t ngx_http_imrs_init(ngx_http_imrs_loc_conf_t *cf);

  ngx_command_t  ngx_http_imrs_commands[] = {
    { ngx_string("imrs"),
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
      ngx_http_imrs,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },

     ngx_null_command
  };

  ngx_http_module_t  ngx_http_imrs_module_ctx = {
      NULL,                          /* preconfiguration */
      NULL,           /* postconfiguration */

      NULL,                          /* create main configuration */
      NULL,                          /* init main configuration */

      NULL,                          /* create server configuration */
      NULL,                          /* merge server configuration */

      ngx_http_imrs_create_loc_conf,  /* create location configuration */
      ngx_http_imrs_merge_loc_conf /* merge location configuration */
  };

  ngx_module_t ngx_http_imrs_module = {
      NGX_MODULE_V1,
      &ngx_http_imrs_module_ctx, /* module context */
      ngx_http_imrs_commands,   /* module directives */
      NGX_HTTP_MODULE,               /* module type */
      NULL,                          /* init master */
      NULL,                          /* init module */
      NULL,                          /* init process */
      NULL,                          /* init thread */
      NULL,                          /* exit thread */
      NULL,                          /* exit process */
      NULL,                          /* exit master */
      NGX_MODULE_V1_PADDING
  };

};

#define NGX_HTTP_IMAGE_NONE -1
#define NGX_HTTP_IMAGE_JPEG 0
#define NGX_HTTP_IMAGE_GIF  1
#define NGX_HTTP_IMAGE_PNG  2

#endif
