char*
  ngx_http_imrs_cache(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);


#if(NGX_HTTP_CACHE)
    { ngx_string("imrs_cache"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_TAKE1,
      ngx_http_imrs_cache,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL
    },

    //This should be merged into one conf name
    { ngx_string("imrs_cache_path"),
      NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_2MORE,
      ngx_http_file_cache_set_slot,
      0,
      0,
      &ngx_http_imrs_module },
#endif

