#ifndef IMRS_RESP_COMP
#define IMRS_RESP_COMP

extern "C" {
  #include <ngx_config.h>
  #include <ngx_core.h>
  #include <ngx_http.h>
}

class ImrsResponse{
  protected:
    typedef struct {
      char* data;
      size_t size;
    } data;

  public:
    virtual void writeData(void*, size_t, size_t, void*) = 0;
};

#endif
