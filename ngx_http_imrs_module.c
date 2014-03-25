/*
 * Copyright (C) Evan Miller
 */

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <ngx_md5.h>
#include <curl/curl.h>
//#include <wand/MagickWand.h>
#include <opencv/cv.h>

#define NGX_HTTP_IMAGE_NONE -1
#define NGX_HTTP_IMAGE_JPEG 0
#define NGX_HTTP_IMAGE_GIF  1
#define NGX_HTTP_IMAGE_PNG  2

ngx_module_t  ngx_http_imrs_module;

static ngx_str_t image_types[] = {
  ngx_string("image/jpeg"),
  ngx_string("image/gif"),
  ngx_string("image/png")
};

//What do the levels do...?
static ngx_path_init_t ngx_http_imrs_default_path = {
  ngx_string("imrs"), { 1, 2, 0 }
};

typedef struct {
  ngx_shm_zone_t    *cache;
} ngx_http_imrs_loc_conf_t;

typedef struct {
  ngx_str_t    src;
  ngx_str_t    action;
  size_t       h;
  size_t       w;
  size_t       offx;
  size_t       offy;
  ngx_str_t    quality;
} ReqImage;

typedef struct {
  FILE*       file_to_write;
  char*       image_data;
  ngx_str_t   image_type;
  size_t      image_len;
  Image*      image;
} RespImage;

typedef struct {
  char*     data;
  size_t    size;
} RawImage;

static char* ngx_http_imrs(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);
static void* ngx_http_imrs_create_loc_conf(ngx_conf_t *cf);
static char* ngx_http_imrs_merge_loc_conf(ngx_conf_t *cf,
    void *parent, void *child);
static ngx_int_t ngx_http_imrs_init_module(ngx_cycle_t *cycle);
static ngx_int_t ngx_http_imrs_init(ngx_http_imrs_loc_conf_t *cf);
static ngx_uint_t
ngx_http_image_test(char* image_buf);
static char*
ngx_http_imrs_cache(ngx_conf_t *cf, ngx_command_t *cmd, void *conf);

static ngx_command_t  ngx_http_imrs_commands[] = {
    { ngx_string("imrs"),
      NGX_HTTP_LOC_CONF|NGX_CONF_NOARGS,
      ngx_http_imrs,
      NGX_HTTP_LOC_CONF_OFFSET,
      0,
      NULL },
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
    ngx_null_command
};


static ngx_http_module_t  ngx_http_imrs_module_ctx = {
    NULL,                          /* preconfiguration */
    NULL,           /* postconfiguration */

    NULL,                          /* create main configuration */
    NULL,                          /* init main configuration */

    NULL,                          /* create server configuration */
    NULL,                          /* merge server configuration */

    ngx_http_imrs_create_loc_conf,  /* create location configuration */
    ngx_http_imrs_merge_loc_conf /* merge location configuration */
};


ngx_module_t  ngx_http_imrs_module = {
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

static size_t write_image_to_cache(char *img_data,
                  size_t size, size_t nmemb, RawImage* img) {
  ngx_memcpy( img->data + img->size, img_data, size*nmemb );
  img->size += size*nmemb;
  return size * nmemb;
}

//Curl should handle the writing to file.
static char ngx_http_imrs_download_save_image( RawImage* img, ngx_str_t src ) {
    CURL  *curl_h;
    char* url;
    char *curl_err;
    size_t curl_resp;

    url = (char*)malloc(src.len);

    ngx_memcpy(url, src.data, src.len);

    curl_global_init( CURL_GLOBAL_ALL );
    curl_h = curl_easy_init();

    //Whelp, something went wrong.
    if( curl_h == NULL ) {
      return 0;
    }

    //curl_easy_setopt( curl_h, CURLOPT_VERBOSE, 1 );
    curl_easy_setopt( curl_h, CURLOPT_HEADER, 0 );
    curl_easy_setopt( curl_h, CURLOPT_TIMEOUT, 5 );
    curl_easy_setopt( curl_h, CURLOPT_WRITEHEADER, 0 );
    curl_easy_setopt( curl_h, CURLOPT_WRITEFUNCTION, write_image_to_cache );
    curl_easy_setopt( curl_h, CURLOPT_WRITEDATA, img );
    curl_easy_setopt( curl_h, CURLOPT_URL, src.data );
    curl_resp = curl_easy_perform( curl_h );
    curl_global_cleanup();

    free(url);
    return !curl_resp;
}

static ngx_int_t
ngx_setup_incoming_image( ngx_http_request_t *r, ReqImage* img ) {
    ngx_str_t w,h;
    //Need to intialize all these strings to null.
    img->src = (ngx_str_t)ngx_string('\0');
    img->action = (ngx_str_t)ngx_string('\0');
    w = (ngx_str_t)ngx_string('\0');
    h = (ngx_str_t)ngx_string('\0');

    if (ngx_http_arg(r, (u_char *) "src", 3, &img->src) != NGX_OK) {
      return NGX_ERROR;
    }

    ngx_http_arg(r, (u_char*) "action", 6, &img->action);

    if( img->action.data == NULL ) {
      img->action = (ngx_str_t)ngx_string("resize");
    }

    ngx_http_arg(r, (u_char*) "h", 1, &h);

    if( h.data == NULL ) {
      img->h = 100;
    } else {
      img->h = ngx_atoi( h.data, h.len);
    }

    ngx_http_arg(r, (u_char*) "w", 1, &w);

    if( w.data == NULL ) {
      img->w = 100;
    } else {
      img->w = ngx_atoi( w.data, w.len );
    }

    return NGX_OK;
}

static ngx_int_t
ngx_http_imrs_handler(ngx_http_request_t *r)
{
    ngx_int_t                 rc;
    ngx_buf_t                 *b;
    ngx_chain_t               out;
    MagickWand*               wand;
    MagickBooleanType         status;
    ExceptionType             exception_error_code;
    ReqImage                  incomingImage;
    RespImage                 outgoingImage;
    RawImage                  rawImage;
    Image                     *image;
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

    if( ngx_setup_incoming_image(r, &incomingImage) != NGX_OK ) {
      return rc;
    }

    rawImage.data = ngx_palloc( r->pool, 2 * 1024 * 1024);
    rawImage.size = 0;

    if(!ngx_http_imrs_download_save_image(&rawImage, incomingImage.src)) {
        return rc;
    }

    MagickWandGenesis();

    //Ok, let's create a wand
    wand = NewMagickWand();

    //We've read in an image.
    status = MagickReadImageBlob(wand, rawImage.data, rawImage.size );
    if( status == MagickFalse ) {
        MagickRelinquishMemory(wand);
        return rc;
    }

    //Find the next largest aspect ratio above the one we've got.
    int width, height, delta_height, delta_width, ratio, new_height, new_width, x, y;
    width = MagickGetImageWidth(wand);
    height = MagickGetImageHeight(wand);

    delta_width = width - incomingImage.w;
    delta_height = height - incomingImage.h;

    if( delta_width > delta_height ) {
      new_width = incomingImage.h * ( width/height);
      new_height = height / ( width * new_width );
      x = (new_width - incomingImage.w)/2;
      y = 0;
    } else if( delta_height > delta_width ) {
      new_height = incomingImage.w * (height/width);
      new_width = width / (height * new_height);
      y = (new_height - incomingImage.h)/2;
      x = 0;
    } else {
      new_width = incomingImage.w;
      new_height = incomingImage.h;
      x = 0;
      y = 0;
    }

    status = MagickAdaptiveResizeImage(wand, new_width, new_height);
    status = MagickCropImage(wand, incomingImage.w, incomingImage.h, x, y);

    if( status == MagickFalse ) {
      MagickRelinquishMemory(wand);
      return rc;
    }

    MagickSetImageCompressionQuality(wand,85);

    if( status == MagickFalse ) {
      MagickRelinquishMemory(wand);
      return rc;
    }

    outgoingImage.image = GetImageFromMagickWand(wand);
    outgoingImage.image_len = outgoingImage.image->rows * outgoingImage.image->columns;// * outgoingImage.image->depth;
    outgoingImage.image_data = MagickGetImageBlob(wand, &outgoingImage.image_len);
    outgoingImage.image_type = (ngx_str_t)ngx_string(MagickGetImageFormat(wand));

    MagickRelinquishMemory(wand);
    MagickWandTerminus();


    if( strcmp("PNG", outgoingImage.image_type.data) == 0 ) {
      outgoingImage.image_type = image_types[NGX_HTTP_IMAGE_PNG];
    } else if( strcmp( "JPEG", outgoingImage.image_type.data) == 0) {
      outgoingImage.image_type = image_types[NGX_HTTP_IMAGE_JPEG];
    } else {
      return rc;
    }

    r->headers_out.content_type.len = outgoingImage.image_type.len;
    r->headers_out.content_type.data = outgoingImage.image_type.data;
    r->headers_out.status = NGX_HTTP_OK;
    r->headers_out.content_length_n = outgoingImage.image_len;

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

    b->pos = outgoingImage.image_data;
    b->last = outgoingImage.image_data + outgoingImage.image_len;
    b->memory = 1;
    b->last_buf = 1;

    rc = ngx_http_send_header(r);

    if (rc == NGX_ERROR || rc > NGX_OK || r->header_only) {
        return rc;
    }

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
#if(NGX_HTTP_CACHE)
static char*
ngx_http_imrs_cache(ngx_conf_t *cf, ngx_command_t *cmd, void *conf) {
    ngx_http_imrs_loc_conf_t *imclf = conf;
    ngx_str_t *value;

    value = cf->args->elts;

    if( imclf->cache != NGX_CONF_UNSET_PTR) {
        return "is duplicate";
    }

    if(ngx_strcmp(value[1].data, "off") == 0) {
        imclf->cache = NULL;
        return NGX_CONF_OK;
    }

    imclf->cache = ngx_shared_memory_add(cf, &value[1], 0,
                                                 &ngx_http_imrs_module);

    if( imclf->cache == NULL ) {
      return NGX_CONF_ERROR;
    }

    return NGX_CONF_OK;
}
#endif
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
    #if( NGX_HTTP_CACHE )
      conf->cache = NGX_CONF_UNSET_PTR;
    #endif
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

    #if (NGX_HTTP_CACHE)

    ngx_conf_merge_ptr_value(conf->cache,
                              prev->cache, NULL);

    if (conf->cache && conf->cache->data == NULL) {
        ngx_shm_zone_t  *shm_zone;

        //Let's crack open a shared memory zone.
        shm_zone = conf->cache;

        //not good.
        ngx_conf_log_error(NGX_LOG_EMERG, cf, 0,
                           "\"imrs_cache\" zone \"%V\" is unknown",
                           &shm_zone->shm.name);

        return NGX_CONF_ERROR;
    }

    #endif

    return NGX_CONF_OK;
}
