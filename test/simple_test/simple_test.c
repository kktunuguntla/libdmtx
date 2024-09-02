/**
 * libdmtx - Data Matrix Encoding/Decoding Library
 * Copyright 2008, 2009 Mike Laughton. All rights reserved.
 * Copyright 2010-2016 Vadim A. Misbakh-Soloviov. All rights reserved.
 * Copyright 2016 Tim Zaman. All rights reserved.
 *
 * See LICENSE file in the main project directory for full
 * terms of use and distribution.
 *
 * Contact:
 * Vadim A. Misbakh-Soloviov <dmtx@mva.name>
 * Mike Laughton <mike@dragonflylogic.com>
 *
 * \file simple_test.c
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include "../../dmtx.h"
#include <png.h>
// #include <opencv2/opencv.hpp>

// Function to find the intersection point of a line with a scanline
int findIntersection(int y, int y1, int x1, int y2, int x2)
{
   if (y1 == y2)
   {
      return x1;
   }
   return x1 + (y - y1) * (x2 - x1) / (y2 - y1);
}

// Function to traverse pixels within a quadrilateral
void traverseQuadrilateral(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, DmtxRegion *reg, DmtxDecode *dec)
{
   FILE *file = fopen("output_fitx_fity_color_axial_non_uniform.txt", "w");
   if (file == NULL)
   {
      fprintf(stderr, "Error opening file.\n");
      return;
   }
   DmtxVector2 p;
   int colorTmp, colorPlane = 0;
   printf("x1: %d, y1: %d\t x2: %d, y2: %d\t x3: %d, y3: %d\t x4: %d, y4: %d\n", x1, y1, x2, y2, x3, y3, x4, y4);
   int minY = y1 < y2 ? (y1 < y3 ? y1 : y3) : (y2 < y3 ? y2 : y3);
   int maxY = y4 > y2 ? (y4 > y3 ? y4 : y3) : (y2 > y3 ? y2 : y3);
   int counter = 0;
   printf("minY: %d\t maxY: %d\n", minY, maxY);
   for (int y = minY; y <= maxY; ++y)
   {
      int xStart = findIntersection(y, y1, x1, y2, x2);
      int xEnd = findIntersection(y, y3, x3, y4, x4);
      // Loop through columns
      for (int x = xStart; x <= xEnd; ++x)
      {
         // Process each pixel at coordinates (x, y)
         // printf("(%d, %d)", x, y);
         p.X = (float)x;
         p.Y = (float)y;
         counter++;
         if (counter == 25741)
         {
            break;
         }
         printf("%d\n", counter);
         // get color value
         dmtxDecodeGetPixelValue(dec, (int)(p.X), (int)(p.Y),
                                 colorPlane, &colorTmp);
         // fprintf(file, "%d, %d, %d \n", (int)(p.X), (int)(p.Y), colorTmp);
         dmtxMatrix3VMultiplyBy(&p, reg->raw2fit);
         fprintf(file, "%f, %f, %d \n", p.X, p.Y, colorTmp);
         // printf(" (%f, %f) %d\t", p.X, p.Y, colorTmp);
      }
   }
}

int main(int argc, char *argv[])
{
   // size_t width, height, bytesPerPixel;
   // unsigned char str[] = "30Q324343430794<OQQ";
   // unsigned char *pxl;
   DmtxEncode *enc;
   DmtxImage *img;
   DmtxDecode *dec;
   DmtxRegion *reg;
   DmtxMessage *msg;
   int colorTmp, colorPlane = 0;
   // fprintf(stdout, "input:  \"%s\"\n", str);

   /* 1) ENCODE a new Data Matrix barcode image (in memory only) */

   // enc = dmtxEncodeCreate();

   /*
    dmtxEncodeSetProp( enc, DmtxPropPixelPacking, DmtxPack16bppRGB );
    dmtxEncodeSetProp( enc, DmtxPropPixelPacking, DmtxPack32bppRGB );
    dmtxEncodeSetProp( enc, DmtxPropWidth, 160 );
    dmtxEncodeSetProp( enc, DmtxPropHeight, 160 );
   */

   // assert(enc != NULL);
   // dmtxEncodeDataMatrix(enc, strlen((const char *)str), str);

   // /* 2) COPY the new image data before releasing encoding memory */

   // width = dmtxImageGetProp(enc->image, DmtxPropWidth);
   // height = dmtxImageGetProp(enc->image, DmtxPropHeight);
   // bytesPerPixel = dmtxImageGetProp(enc->image, DmtxPropBytesPerPixel);

   // pxl = (unsigned char *)malloc(width * height * bytesPerPixel);
   // assert(pxl != NULL);
   // memcpy(pxl, enc->image->pxl, width * height * bytesPerPixel);

   // dmtxEncodeDestroy(&enc);

   FILE *fp = fopen("./images/datamatrix_2d_8.png", "rb");
   if (!fp)
   {
      perror("File opening failed");
      return EXIT_FAILURE;
   }

   png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
   if (!png_ptr)
      abort();

   png_infop info_ptr = png_create_info_struct(png_ptr);
   if (!info_ptr)
      abort();

   if (setjmp(png_jmpbuf(png_ptr)))
      abort();

   png_init_io(png_ptr, fp);

   png_read_info(png_ptr, info_ptr);

   // // // Add your code here to process the PNG image
   // // // For example, get image width and height
   int width = png_get_image_width(png_ptr, info_ptr);
   int height = png_get_image_height(png_ptr, info_ptr);
   int color_type = png_get_color_type(png_ptr, info_ptr);
   int bit_depth = png_get_bit_depth(png_ptr, info_ptr);
   int top_left_cordinates[2];
   int top_right_cordinates[2];
   int bottom_left_cordinates[2];
   int bottom_right_cordinates[2];

   int bytesPerPixel;
   switch (color_type)
   {
   case PNG_COLOR_TYPE_RGB:
      bytesPerPixel = 3 * bit_depth / 8;
      break;
   case PNG_COLOR_TYPE_RGBA:
      bytesPerPixel = 4 * bit_depth / 8;
      break;
   case PNG_COLOR_TYPE_GRAY:
   case PNG_COLOR_TYPE_PALETTE:
      bytesPerPixel = bit_depth / 8;
      break;
   // Add other color types if necessary
   default:
      fprintf(stderr, "Unsupported color type\n");
      return EXIT_FAILURE;
   }

   unsigned char *pxl = (unsigned char *)malloc(width * height * bytesPerPixel);

   if (!pxl)
   {
      fprintf(stderr, "Memory allocation failed\n");
      return EXIT_FAILURE;
   }

   fprintf(stdout, "width:  \"%d\"\n", width);
   fprintf(stdout, "height: \"%d\"\n", height);
   fprintf(stdout, "bpp:    \"%d\"\n", bytesPerPixel);

   int bytesPerRow = png_get_rowbytes(png_ptr, info_ptr);
   // unsigned char *pxl = (unsigned char *)malloc(height * bytesPerRow);
   // if (!pxl)
   //    return EXIT_FAILURE;

   // Allocate memory for row pointers
   png_bytep *row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
   for (int y = 0; y < height; y++)
   {
      row_pointers[y] = pxl + y * bytesPerRow;
   }

   png_read_image(png_ptr, row_pointers);

   // for (int i = 0; i < width * height; i++)
   // {
   //    fprintf(stdout, "%u\t", (pxl[i * 3]));
   //    if (i % width == width - 1)
   //    {
   //       fprintf(stdout, "\n");
   //    }
   // }
   // fprintf(stdout, "\n\n\n");

   // for (int i = 0; i < width * height; i++)
   // {
   //    fprintf(stdout, "%d", (pxl[i * 3]) == 0);
   //    if (i % width == width - 1)
   //    {
   //       fprintf(stdout, "\n");
   //    }
   // }
   // fprintf(stdout, "\n\n\n");

   /* 3) DECODE the Data Matrix barcode from the copied image */

   img = dmtxImageCreate(pxl, width, height, DmtxPack24bppRGB);
   assert(img != NULL);
   fprintf(stdout, "Dmtx Image created \n");

   dec = dmtxDecodeCreate(img, 1);
   assert(dec != NULL);
   fprintf(stdout, "Preprocessed image: \n");
   // DmtxImage *image = dec->image;
   // unsigned char *pixel = image->pxl;
   // for (int i=0; i<width*height; i++){
   //    fprintf(stdout, "%d", (pixel[i*3])==0);
   //    if (i%width==width-1){
   //       fprintf(stdout, "\n");
   //    }
   // } 
   reg = dmtxRegionFindNext(dec, NULL);
   fprintf(stdout, "reg->symbolRows: \"%d\"\n", reg->symbolRows);
   fprintf(stdout, "reg->symbolCols: \"%d\"\n", reg->symbolCols);
   fprintf(stdout, "reg->mappingRows: \"%d\"\n", reg->mappingRows);
   fprintf(stdout, "reg->mappingCols: \"%d\"\n", reg->mappingCols);

   DmtxVector2 p;
   for (int i = 1; i < reg->symbolCols + 1; i++)
   {
      printf("\n");
      for (int j = 1; j < reg->symbolRows; j++)
      {
         p.X = ((1.0 / reg->symbolCols) * (j - 0.8));
         p.Y = (1.0 / reg->symbolRows) * (17 - i);
         dmtxMatrix3VMultiplyBy(&p, reg->fit2raw);
         // printf("%dx%d \n", (int)(p.X), (int)(p.Y));
         dmtxDecodeGetPixelValue(dec, (int)(p.X + 0.5), (int)(p.Y + 0.5),
                                 colorPlane, &colorTmp);
         printf("%dx%d", (int)(p.X + 0.5), (int)(p.Y + 0.5));
         printf("  %d \t", colorTmp);
      }
   }
   DmtxVector2 p1;
   p1.X = 0.0;
   p1.Y = 0.0;
   dmtxMatrix3VMultiplyBy(&p1, reg->fit2raw);
   // printf("%dx%d \n", (int)(p.X), (int)(p.Y));
   bottom_left_cordinates[0] = (int)(p1.X);
   bottom_left_cordinates[1] = (int)(p1.Y);
   printf("\nBottom left cordinates: %d, %d\n", bottom_left_cordinates[1], bottom_left_cordinates[0]);

   p1.X = 0.0;
   p1.Y = 1.0;
   dmtxMatrix3VMultiplyBy(&p1, reg->fit2raw);
   // printf("%dx%d \n", (int)(p.X), (int)(p.Y));
   top_left_cordinates[0] = (int)(p1.X);
   top_left_cordinates[1] = (int)(p1.Y);
   printf("\nTop left cordinates: %d, %d\n", top_left_cordinates[0], top_left_cordinates[1]);

   p1.X = 1.0;
   p1.Y = 0.0;
   dmtxMatrix3VMultiplyBy(&p1, reg->fit2raw);
   // printf("%dx%d \n", (int)(p.X), (int)(p.Y));
   bottom_right_cordinates[0] = (int)(p1.X);
   bottom_right_cordinates[1] = (int)(p1.Y);
   printf("\nBottom right cordinates: %d, %d\n", bottom_right_cordinates[0], bottom_right_cordinates[1]);

   p1.X = 1.0;
   p1.Y = 1.0;
   dmtxMatrix3VMultiplyBy(&p1, reg->fit2raw);
   // printf("%dx%d \n", (int)(p.X), (int)(p.Y));
   top_right_cordinates[0] = (int)(p1.X);
   top_right_cordinates[1] = (int)(p1.Y);
   printf("\nTop right cordinates: %d, %d\n", top_right_cordinates[0], top_right_cordinates[1]);

   // traverseQuadrilateral(bottom_left_cordinates[0], bottom_left_cordinates[1], top_left_cordinates[0], top_left_cordinates[1], bottom_right_cordinates[0], bottom_right_cordinates[1], top_right_cordinates[0], top_right_cordinates[1], reg, dec);
   if (reg != NULL)
   {
      msg = dmtxDecodeMatrixRegion(dec, reg, DmtxUndefined);
      if(msg == NULL){
         printf("Message is NULL\n");
      }
      else{
         printf("Message is not NULL\n");
      }
      fprintf(stdout, "msg->arraySize :  \"%zd\"\n", msg->arraySize);
      fprintf(stdout, "msg->codeSize  :  \"%zd\"\n", msg->codeSize);
      fprintf(stdout, "msg->outputSize:  \"%zd\"\n", msg->outputSize);
      fprintf(stdout, "msg->uec :  \"%f\"\n", *msg->uec);

      int oned = sqrt(msg->arraySize);
      for (int i = 0; i < msg->arraySize; i++)
      {
         fprintf(stdout, " %c.", msg->array[i]);
         if (i % oned == oned - 1)
         {
            fprintf(stdout, "\n");
         }
      }
      fprintf(stdout, "\n\n");

      // for (int i = 0; i < 14; i++)
      // {
      //    printf("\n");
      //    for (int j = 0; j < 14; j++)
      //    {
      //       printf(" %u", msg->array[i * reg->symbolRows + j]);
      //    }
      // }

      // fprintf(stdout, "\n\n");

      // for (int i = 0; i < msg->arraySize; i++)
      // {
      //    // fprintf(stdout, " %d.", msg->intensityArray[i]);
      //    if (i % oned == oned - 1)
      //    {
      //       fprintf(stdout, "\n");
      //    }
      // }
      // fprintf(stdout, "\n\n");

      // for (int j = 0; j < msg->codeSize; j++)
      // {
      //    fprintf(stdout, " %c.", msg->code[j]);
      // }
      // fprintf(stdout, "\n\n");
      // for (int k = 0; k < msg->outputSize; k++)
      // {
      //    fprintf(stdout, " %c.", msg->output[k]);
      // }
      // fprintf(stdout, "\n\n");

      if (msg != NULL)
      {
         fputs("output: \"", stdout);
         fwrite(msg->output, sizeof(unsigned char), msg->outputIdx, stdout);
         fputs("\"\n", stdout);
         dmtxMessageDestroy(&msg);
      }
      dmtxRegionDestroy(&reg);
   }
   else{
   fprintf(stdout, "Decoding Failed \n");
   }

   dmtxDecodeDestroy(&dec);
   dmtxImageDestroy(&img);
   free(pxl); 

   fprintf(stdout, "%d\n", getSizeIdxFromSymbolDimension(12, 12));

   exit(0);
}
