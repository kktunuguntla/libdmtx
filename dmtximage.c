/*
libdmtx - Data Matrix Encoding/Decoding Library

Copyright (C) 2008, 2009 Mike Laughton

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

Contact: mike@dragonflylogic.com
*/

/* $Id$ */

/**
 * @file dmtximage.c
 * @brief Image handling
 */

/**
 * libdmtx treats image data as a single 1D array of packed pixels. When
 * reading and writing barcodes, this array provides the sole mechanism for
 * pixel storage and libdmtx relies on the calling program to transfer
 * images to/from the outside world (e.g., saving to disk, acquiring camera
 * input, etc...).
 *
 * By default, libdmtx treats the first pixel of an array as the top-left
 * location of an image, with horizontal rows working downward to the
 * final pixel at the bottom-right corner. If mapping a pixel buffer this
 * way produces an inverted image, then specify DmtxFlipY at image
 * creation time to remove the inversion. Note that DmtxFlipY has no
 * significant affect on performance since it only modifies the pixel
 * mapping math and does not alter any pixel data. If the image appears
 * correctly without any flips then specify DmtxFlipNone.
 *
 * Regardless of how an image is stored internally, all libdmtx functions
 * consider coordinate (x=0,y=0) to represent the bottom-left pixel
 * location of an image.
 *
 *                (0,HEIGHT-1)        (WIDTH-1,HEIGHT-1)
 *
 *          array pos = 0,1,2,3,...-----------+
 *                      |                     |
 *                      |                     |
 *                      |       libdmtx       |
 *                      |        image        |
 *                      |     coordinates     |
 *                      |                     |
 *                      |                     |
 *                      +---------...,N-2,N-1,N = array pos
 *
 *                    (0,0)              (WIDTH-1,0)
 *
 * Notes:
 *   - OpenGL pixel arrays obtained with glReadPixels() are stored
 *     bottom-to-top; use DmtxFlipY
 *   - Many popular image formats (e.g., PNG, GIF) store rows
 *     top-to-bottom; use DmtxFlipNone
 */

/**
 * @brief  xxx
 * @param  xxx
 * @return xxx
 */
extern DmtxImage *
dmtxImageCreate(unsigned char *pxl, int width, int height, int pack)
{
   DmtxPassFail err;
   DmtxImage *img;

   if(pxl == NULL || width < 1 || height < 1)
      return NULL;

   img = (DmtxImage *)calloc(1, sizeof(DmtxImage));
   if(img == NULL)
      return NULL;

   img->pxl = pxl;
   img->width = width;
   img->height = height;
   img->pixelPacking = pack;
   img->imageFlip = DmtxFlipNone;
   img->channelCount = 0;

   /* XXX later these settings move to DmtxDecode */
   img->widthScaled = width;
   img->heightScaled = height;
   img->scale = 1;
   img->xMin = img->xMinScaled = 0;
   img->xMax = img->xMaxScaled = width - 1;
   img->yMin = img->yMinScaled = 0;
   img->yMax = img->yMaxScaled = height - 1;

   switch(pack) {
      case DmtxPackCustom:
         break;
      case DmtxPack1bppK:
         img->bitsPerPixel = 1;
         break;
      case DmtxPack8bppK:
         img->bitsPerPixel = 8;
         break;
      case DmtxPack16bppRGB:
      case DmtxPack16bppRGBX:
      case DmtxPack16bppXRGB:
      case DmtxPack16bppBGR:
      case DmtxPack16bppBGRX:
      case DmtxPack16bppXBGR:
      case DmtxPack16bppYCbCr:
         img->bitsPerPixel = 16;
         break;
      case DmtxPack24bppRGB:
      case DmtxPack24bppBGR:
      case DmtxPack24bppYCbCr:
         img->bitsPerPixel = 24;
         break;
      case DmtxPack32bppRGBX:
      case DmtxPack32bppXRGB:
      case DmtxPack32bppBGRX:
      case DmtxPack32bppXBGR:
      case DmtxPack32bppCMYK:
         img->bitsPerPixel = 32;
         break;
      default:
         return NULL;
   }

   switch(pack) {
      case DmtxPackCustom:
         break;
      case DmtxPack1bppK:
         err = dmtxImageSetChannel(img, 0, 1);
         break;
      case DmtxPack8bppK:
         err = dmtxImageSetChannel(img, 0, 8);
         break;
      case DmtxPack16bppRGB:
      case DmtxPack16bppBGR:
      case DmtxPack16bppYCbCr:
         err = dmtxImageSetChannel(img,  0, 5);
         err = dmtxImageSetChannel(img,  5, 5);
         err = dmtxImageSetChannel(img, 10, 5);
         break;
      case DmtxPack24bppRGB:
      case DmtxPack24bppBGR:
      case DmtxPack24bppYCbCr:
      case DmtxPack32bppRGBX:
      case DmtxPack32bppBGRX:
         err = dmtxImageSetChannel(img,  0, 8);
         err = dmtxImageSetChannel(img,  8, 8);
         err = dmtxImageSetChannel(img, 16, 8);
         break;
      case DmtxPack16bppRGBX:
      case DmtxPack16bppBGRX:
         err = dmtxImageSetChannel(img,  0, 5);
         err = dmtxImageSetChannel(img,  5, 5);
         err = dmtxImageSetChannel(img, 10, 5);
         break;
      case DmtxPack16bppXRGB:
      case DmtxPack16bppXBGR:
         err = dmtxImageSetChannel(img,  1, 5);
         err = dmtxImageSetChannel(img,  6, 5);
         err = dmtxImageSetChannel(img, 11, 5);
         break;
      case DmtxPack32bppXRGB:
      case DmtxPack32bppXBGR:
         err = dmtxImageSetChannel(img,  8, 8);
         err = dmtxImageSetChannel(img, 16, 8);
         err = dmtxImageSetChannel(img, 24, 8);
         break;
      case DmtxPack32bppCMYK:
         err = dmtxImageSetChannel(img,  0, 8);
         err = dmtxImageSetChannel(img,  8, 8);
         err = dmtxImageSetChannel(img, 16, 8);
         err = dmtxImageSetChannel(img, 24, 8);
         break;
      default:
         return NULL;
   }

   return img;
}

/**
 * @brief  Free libdmtx image memory
 * @param  img pointer to img location
 * @return DmtxFail | DmtxPass
 */
extern DmtxPassFail
dmtxImageDestroy(DmtxImage **img)
{
   if(img == NULL || *img == NULL)
      return DmtxFail;

   free(*img);

   *img = NULL;

   return DmtxPass;
}

/**
 *
 *
 */
extern DmtxPassFail
dmtxImageSetChannel(DmtxImage *img, int channelStart, int bitsPerChannel)
{
   if(img->channelCount >= 4) /* IMAGE_MAX_CHANNEL */
      return DmtxFail;

   /* New channel extends beyond pixel data */
/* if(channelStart + bitsPerChannel > img->bitsPerPixel)
      return DmtxFail; */

   img->bitsPerChannel[img->channelCount] = bitsPerChannel;
   img->channelStart[img->channelCount] = channelStart;
   (img->channelCount)++;

   return DmtxPass;
}

/**
 * @brief  Set image property
 * @param  img pointer to image
 * @return image width
 */
extern DmtxPassFail
dmtxImageSetProp(DmtxImage *img, int prop, int value)
{
   if(img == NULL)
      return DmtxFail;

   switch(prop) {
      case DmtxPropWidth:
         img->width = value;
         img->widthScaled = img->width/img->scale;
         break;
      case DmtxPropHeight:
         img->height = value;
         img->heightScaled = img->height/img->scale;
         break;
      case DmtxPropImageFlip:
         img->imageFlip = value;
         break;
      case DmtxPropRowPadBytes:
         img->rowPadBytes = value;
         break;
      case DmtxPropXmin:
         img->xMin = value;
         img->xMinScaled = img->xMin/img->scale;
         break;
      case DmtxPropXmax:
         img->xMax = value;
         img->xMaxScaled = img->xMax/img->scale;
         break;
      case DmtxPropYmin: /* Deliberate y-flip */
         img->yMax = img->height - value - 1;
         img->yMaxScaled = img->yMax/img->scale;
         break;
      case DmtxPropYmax: /* Deliberate y-flip */
         img->yMin = img->height - value - 1;
         img->yMinScaled = img->yMin/img->scale;
         break;
      case DmtxPropScale:
         img->scale = value;
         img->widthScaled = img->width/value;
         img->heightScaled = img->height/value;
         img->xMinScaled = img->xMin/value;
         img->xMaxScaled = img->xMax/value;
         img->yMinScaled = img->yMin/value;
         img->yMaxScaled = img->yMax/value;
         break;

      default:
         return DmtxFail;
   }

   /* Specified range has non-positive area */
   if(img->xMin >= img->xMax || img->yMin >= img->yMax)
      return DmtxFail;

   /* Specified range extends beyond image boundaries */
   if(img->xMin < 0 || img->xMax >= img->width ||
         img->yMin < 0 || img->yMax >= img->height)
      return DmtxFail;

   return DmtxPass;
}

/**
 * @brief  Get image width
 * @param  img pointer to image
 * @return image width
 */
extern int
dmtxImageGetProp(DmtxImage *img, int prop)
{
   if(img == NULL)
      return DmtxUndefined;

   switch(prop) {
      case DmtxPropWidth:
         return img->width;
      case DmtxPropHeight:
         return img->height;
      case DmtxPropBitsPerPixel:
         return img->bitsPerPixel;
      case DmtxPropBytesPerPixel:
         return img->bitsPerPixel/8;
      case DmtxPropArea:
         return img->width * img->height;
      case DmtxPropXmin:
         return img->xMin;
      case DmtxPropXmax:
         return img->xMax;
      case DmtxPropYmin:
         return img->yMin;
      case DmtxPropYmax:
         return img->yMax;
      case DmtxPropScale:
         return img->scale;
      case DmtxPropScaledWidth:
         return img->width/img->scale;
      case DmtxPropScaledHeight:
         return img->height/img->scale;
      case DmtxPropScaledArea:
         return (img->width/img->scale) * (img->height/img->scale);
      case DmtxPropScaledXmin:
         return img->xMinScaled;
      case DmtxPropScaledXmax:
         return img->xMaxScaled;
      case DmtxPropScaledYmin:
         return img->yMinScaled;
      case DmtxPropScaledYmax:
         return img->yMaxScaled;
   }

   return DmtxUndefined;
}

/**
 * @brief  Returns pixel offset for unscaled image
 * @param  img
 * @param  x Scaled x coordinate
 * @param  y Scaled y coordinate
 * @return Unscaled pixel offset
 */
extern int
dmtxImageGetPixelOffset(DmtxImage *img, int x, int y)
{
   int offset;

   assert(img != NULL);
   assert(!(img->imageFlip & DmtxFlipX)); /* not implemented */

   if(dmtxImageContainsInt(img, 0, x, y) == DmtxFalse)
      return DmtxUndefined;

   if(img->imageFlip & DmtxFlipY)
      offset = img->scale * (y * img->width + x);
   else
      offset = ((img->heightScaled - y - 1) * img->scale * img->width + (x * img->scale));

   return offset;
}

/**
 *
 *
 */
extern DmtxPassFail
dmtxImageGetPixelValue(DmtxImage *img, int x, int y, int channel, int *value)
{
   unsigned char *pixelPtr;
   int pixelValue;
   int offset;
   int mask;
   int bitShift;
   int bytesPerPixel;

   assert(img != NULL);
   assert(channel < img->channelCount);

   offset = dmtxImageGetPixelOffset(img, x, y);
   if(offset == DmtxUndefined)
      return DmtxFail;

   switch(img->bitsPerChannel[channel]) {
      case 1:
         assert(img->bitsPerPixel == 1);
         mask = 0x01 << (7 - offset%8);
         *value = (img->pxl[offset/8] & mask) ? 255 : 0;
         break;
      case 5:
         /* XXX might be expensive if we want to scale perfect 0-255 range */
         assert(img->bitsPerPixel == 16);
         pixelPtr = img->pxl + (offset * (img->bitsPerPixel/8));
         pixelValue = (*pixelPtr << 8) | (*(pixelPtr+1));
         bitShift = img->bitsPerPixel - 5 - img->channelStart[channel];
         mask = 0x1f << bitShift;
         *value = (((pixelValue & mask) >> bitShift) << 3);
         break;
      case 8:
         assert(img->channelStart[channel] % 8 == 0);
         assert(img->bitsPerPixel % 8 == 0);
         bytesPerPixel = img->bitsPerPixel / 8;
         *value = img->pxl[offset * bytesPerPixel + channel];
         break;
   }

   return DmtxPass;
}

/**
 *
 *
 */
extern DmtxPassFail
dmtxImageSetPixelValue(DmtxImage *img, int x, int y, int channel, int value)
{
/* unsigned char *pixelPtr; */
/* int pixelValue; */
   int offset;
/* int mask; */
/* int bitShift; */
   int bytesPerPixel;

   assert(img != NULL);
   assert(channel < img->channelCount);

   offset = dmtxImageGetPixelOffset(img, x, y);
   if(offset == DmtxUndefined)
      return DmtxFail;

   switch(img->bitsPerChannel[channel]) {
      case 1:
/*       assert(img->bitsPerPixel == 1);
         mask = 0x01 << (7 - offset%8);
         *value = (img->pxl[offset/8] & mask) ? 255 : 0; */
         break;
      case 5:
         /* XXX might be expensive if we want to scale perfect 0-255 range */
/*       assert(img->bitsPerPixel == 16);
         pixelPtr = img->pxl + (offset * (img->bitsPerPixel/8));
         pixelValue = (*pixelPtr << 8) | (*(pixelPtr+1));
         bitShift = img->bitsPerPixel - 5 - img->channelStart[channel];
         mask = 0x1f << bitShift;
         *value = (((pixelValue & mask) >> bitShift) << 3); */
         break;
      case 8:
         assert(img->channelStart[channel] % 8 == 0);
         assert(img->bitsPerPixel % 8 == 0);
         bytesPerPixel = img->bitsPerPixel / 8;
         img->pxl[offset * bytesPerPixel + channel] = value;
         break;
   }

   return DmtxPass;
}

/**
 * @brief  Test whether image contains a coordinate expressed in integers
 * @param  img
 * @param  margin Unscaled margin width
 * @param  x Scaled x coordinate
 * @param  y Scaled y coordinate
 * @return DmtxTrue | DmtxFalse
 */
extern DmtxBoolean
dmtxImageContainsInt(DmtxImage *img, int margin, int x, int y)
{
   assert(img != NULL);

   if(x - margin >= img->xMinScaled && x + margin <= img->xMaxScaled &&
         y - margin >= img->yMinScaled && y + margin <= img->yMaxScaled)
      return DmtxTrue;

   return DmtxFalse;
}

/**
 * @brief  Test whether image contains a coordinate expressed in floating points
 * @param  img
 * @param  x Scaled x coordinate
 * @param  y Scaled y coordinate
 * @return DmtxTrue | DmtxFalse
 */
extern DmtxBoolean
dmtxImageContainsFloat(DmtxImage *img, double x, double y)
{
   assert(img != NULL);

   if(x >= (double)img->xMinScaled && x <= (double)img->xMaxScaled &&
         y >= (double)img->yMinScaled && y <= (double)img->yMaxScaled)
      return DmtxTrue;

   return DmtxFalse;
}
