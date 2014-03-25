#include "ngx_http_imrs_image.h"

FormattedImage::FormattedImage(char* imgData, size_t imgLen) {
    //Make a VImage
    imgData = (u_char*)imgData;
    tempImg = (VipsImage*)malloc( sizeof(VipsImage) );
    vips_jpegload_buffer( imgData, imgLen, &tempImg, NULL );
    img = new vips::VImage(tempImg);

    //Save the raw data and size
    vips_jpegsave_buffer( img->image(), &tempData, &rawImageLen, NULL );
    rawImageData = (u_char*)tempData;
}

FormattedImage::~FormattedImage() {
  delete img;

  free( tempImg );
  free( tempImg );
  free( tempData );
}

void FormattedImage::resizeImage(int reqWidth, int reqHeight) {
    float width, height, delta_height, delta_width, 
        ratio, x, y;
    int new_width, new_height;

    width = img->Xsize();
    height = img->Ysize();

    delta_width = width - reqWidth;
    delta_height = height - reqHeight;

    x = 0, y = 0;
    if( delta_width > delta_height ) {
      //new_width = (reqHeight * width)/height;
      //new_height = ( width * new_width ) / height;
      x = (new_width - reqWidth)/2;
    } else if( delta_height > delta_width ) {
      //new_height = reqWidth * (height/width);
      //new_width = width / (height * new_height);
      y = (new_height - reqHeight)/2;
    }

    double byPoint = (double)reqWidth/reqHeight;

    vips::VImage resizedImg = img->affine( byPoint, 0, 0, byPoint, 0, 0, x, y, reqWidth, reqHeight );
    img = new vips::VImage( resizedImg );

    //Store the data we've retrieved back into a jpeg-style buffer.
    vips_jpegsave_buffer( img->image(), &tempData, &rawImageLen, NULL );
    rawImageData = (u_char*)tempData;
}

float FormattedImage::getAverage() {
  return img->avg();
}

int FormattedImage::getWidth() {
  return img->Xsize();
}

int FormattedImage::getHeight() {
  return img->Ysize();
}

u_char* FormattedImage::getData() {
  return (u_char*)img->data();
}

u_char* FormattedImage::getRawData() {
  return rawImageData;
}

size_t FormattedImage::getRawLen() {
  return rawImageLen;
}

int FormattedImage::getSize() {
  return (img->Xsize() * img->Ysize() * img->Bands())/10;
}

VipsImage* FormattedImage::getImage() {
  return img->image();
}

void FormattedImage::write( char* filename ) {
  img->write( filename );
}
