/*
 * Copyright (C) Evan Miller
 */

#include "ngx_http_imrs_module.h"
#include "fetch/ngx_http_imrs_curl.h"
#include "image/ngx_http_imrs_raw_image.h"
#include "image/ngx_http_imrs_formatted_image.h"

static ngx_int_t
ngx_http_imrs_handler(ngx_http_request_t *r)
{
    ngx_int_t    rc;
    ngx_buf_t    *b;
    ngx_chain_t  out;
    RequestImage reqImg(r);
    ImrsImage* img;
    ImrsCurl imgReq;

    ngx_http_imrs_loc_conf_t  *imrslcf;

    imrslcf = (ngx_http_imrs_loc_conf_t *)ngx_http_get_module_loc_conf(r, ngx_http_imrs_module);

    if (!(r->method & (NGX_HTTP_GET|NGX_HTTP_HEAD))) {
        return NGX_HTTP_NOT_ALLOWED;
    }

    rc = ngx_http_discard_request_body(r);

    if (rc != NGX_OK && rc != NGX_AGAIN) {
        return rc;
    }

    if (r->headers_in.if_modified_since) {
        return NGX_HTTP_NOT_MODIFIED;
    }

    if (!r->args.len)
      return rc;

    //RawImage rawImg(reqImg.getSource());
    //rawImg.downloadImage();
    imgReq = new ImrsCurl(reqImg.getSource());
    imgReq.downloadImg();
    formattedImage = new FormattedImage(imgR->data->data, imgR->data->size);
    formattedImage->resizeImage( reqImg.getWidth(), reqImg.getHeight() );

    u_char* data = formattedImage->getRawData();
    size_t len = formattedImage->getRawLen();
    r->headers_out.content_type.len = image_types[0].len;
    r->headers_out.content_type.data = image_types[0].data;
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = len;

    if (r->method == NGX_HTTP_HEAD) {
        rc = ngx_http_send_header(r);

        if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
            return rc;
        }
    }

    b = (ngx_buf_t *) ngx_pcalloc(r->pool, sizeof(ngx_buf_t));
    if (b == NULL) {
      ngx_log_error(NGX_LOG_ERR, r->connection->log, 0, "Failed to allocate response buffer.");
      return NGX_HTTP_INTERNAL_SERVER_ERROR;
    }

    out.buf = b;
    out.next = NULL;

    b->pos = data;
    b->last = data + len;
    b->memory = 1;
    b->last_buf = 1;

    rc = ngx_http_send_header(r);

    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

    delete( formattedImage );
    delete( imgR );
    free( data );
    return ngx_http_output_filter(r, &out);
}

static char *
ngx_http_imrs(ngx_conf_t *cf, ngx_command_t *cmd, void *conf)
{
    ngx_http_core_loc_conf_t  *clcf;
    ngx_http_imrs_loc_conf_t *imrslcf = (ngx_http_imrs_loc_conf_t * )conf;

    clcf = (ngx_http_core_loc_conf_t *) ngx_http_conf_get_module_loc_conf(cf, ngx_http_core_module);
    clcf->handler = ngx_http_imrs_handler;

    return NGX_CONF_OK;
}

static void *
ngx_http_imrs_create_loc_conf(ngx_conf_t *cf)
{
    ngx_http_imrs_loc_conf_t  *conf;

    conf = (ngx_http_imrs_loc_conf_t *)ngx_pcalloc(cf->pool, sizeof(ngx_http_imrs_loc_conf_t));

    if (conf == NULL) {
        return NGX_CONF_ERROR;
    }

    //Honestly not sure why we need this check (NGX_HTTP_CACHE).
    //Right now the image manipulation is done within
    //the nginx module. Maybe we should eventually proxy it to another service?
    //Anyway, that's why we don't need conf->upstream.cache. There's no upstream.
    return conf;
}

static char *
ngx_http_imrs_merge_loc_conf(ngx_conf_t *cf, void * parent, void * child)
{
    ngx_http_imrs_loc_conf_t *prev = (ngx_http_imrs_loc_conf_t *) parent;
    ngx_http_imrs_loc_conf_t *conf = (ngx_http_imrs_loc_conf_t *) child;

    //# Don't really care about this, gonna let nginx handle the caching. Might want to use
    //this functionality for long term image storing. (Queue it up?)
    //This is very interesting. There's a reason you pass "cf" to this function. Nginx
    //will handle. The ngx_conf_s struct (which is typedefed to ngx_conf_t) has a path
    //variable. In ngx_cycle.c, directories in the cf variable are created on L337 (ngx_create_paths).
    //Guess that's for efficiency reasons? All modules who need cache directories can have them automagically
    //created at a point that's probably the most efficient.
    //ngx_conf_merge_path_value( cf, &conf->cache_dir, prev->cache_dir, &ngx_http_imrs_default_path );

    return NGX_CONF_OK;
}
