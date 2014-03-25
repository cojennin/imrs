#ifndef IMRS_IMAGE_H
#define IMRS_IMAGE_H

extern "C" {
  #include <ngx_config.h>
  #include <ngx_core.h>
}

#include <vips/vips>
#include <vips/vips.h>

class FormattedImage {
    VipsImage* tempImg;
    vips::VImage* img;
    void* tempData;
    u_char* rawImageData;
    size_t rawImageLen;

    FormattedImage();

  public:
    FormattedImage(char* imgData, size_t imgLen);
    ~FormattedImage();

    void resizeImage(int, int);
    void setRawImageData(char*);

    float getAverage();
    int getWidth();
    int getHeight();
    u_char* getData();
    int getSize();
    VipsImage* getImage();
    void write(char*);
    u_char* getRawData();
    size_t getRawLen();
};

#endif
