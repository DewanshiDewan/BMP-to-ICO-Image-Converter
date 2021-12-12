#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define DATA_OFFSET_OFFSET 0x000A
#define WIDTH_OFFSET 0x0012
#define HEIGHT_OFFSET 0x0016
#define BITS_PER_PIXEL_OFFSET 0x001C
#define HEADER_SIZE 14
#define INFO_HEADER_SIZE 40
#define NO_COMPRESION 0
#define MAX_NUMBER_OF_COLORS 0
#define ALL_COLORS_REQUIRED 0

typedef unsigned int int32;
typedef short int16;
typedef unsigned char byte;

//***Inputs*****
//fileName: The name of the file to open 
//***Outputs****
//pixels: A pointer to a byte array. This will contain the pixel data
//colors: A pointer to a byte array.This will contain the color table data(each element in BGR0 form)
//width: An int pointer to store the width of the image in pixels
//height: An int pointer to store the height of the image in pixels
//bitssPerPixel: An int pointer to store the number of bits per pixel that are used in the image
//resx: An int pointer to store the X Resolution of the image in pixels/meter
//resy: An int pointer to store the Y Resolution of the image in pixels/meter
void ReadBMP(const char *fileName,byte **pixels,byte **colors,int32 *width, int32 *height, int16 *bitsPerPixel,int32 *resx,int32 *resy)
{
        //Open the file for reading in binary mode
        FILE *imageFile = fopen(fileName, "rb");
        //Read data offset
        int32 dataOffset;
        fseek(imageFile, DATA_OFFSET_OFFSET, SEEK_SET);
        fread(&dataOffset, 4, 1, imageFile);
        //Read width
        fseek(imageFile, WIDTH_OFFSET, SEEK_SET);
        fread(width, 4, 1, imageFile);
        //Read height
        fseek(imageFile, HEIGHT_OFFSET, SEEK_SET);
        fread(height, 4, 1, imageFile);
        //Read bits per pixel
        fseek(imageFile, BITS_PER_PIXEL_OFFSET, SEEK_SET);
        fread(bitsPerPixel, 2, 1, imageFile);
	//Read Xresolution
	fseek(imageFile,38,SEEK_SET);
       	fread(resx,4,1,imageFile);
	//Read Yresolution
	fseek(imageFile,42,SEEK_SET);
	fread(resy,4,1,imageFile);	
        //Allocate a pixel array
        int32 bytesPerPixel = ((int32)(*bitsPerPixel) / 8);
	//Read color Table if bytesPerPixel<=1(or bitsPerPixel<=8)
	if(bytesPerPixel<= 1)
	{	
		int numberOfColors = pow(2,*bitsPerPixel);
		*colors= (byte*)malloc(4*numberOfColors);
		fseek(imageFile,54,SEEK_SET);
		fread(*colors, 1, 4*numberOfColors,imageFile);
	}
        //Rows are stored bottom-up
        //Each row is padded to be a multiple of 4 bytes. 
        //We calculate the padded row size in bytes
        int paddedRowSize = (int)(4 * ceil((float)(*width) / 4.0f))*(bytesPerPixel);
        //We are not interested in the padded bytes, so we allocate memory just for
        //the pixel data
        int unpaddedRowSize = (*width)*(bytesPerPixel);
        //Total size of the pixel data in bytes
        int totalSize = unpaddedRowSize*(*height);
        *pixels = (byte*)malloc(totalSize);
        //Read the pixel data Row by Row.
        //Data is padded and stored bottom-up
        int i = 0;
        //point to the last row of our pixel array (unpadded)
        byte *currentRowPointer = *pixels+((*height-1)*unpaddedRowSize);
        for (i = 0; i < *height; i++)
        {
                //put file cursor in the next row from top to bottom
	        fseek(imageFile, dataOffset+(i*paddedRowSize), SEEK_SET);
	        //read only unpaddedRowSize bytes (we can ignore the padding bytes)
	        fread(currentRowPointer, 1, unpaddedRowSize, imageFile);
	        //point to the next row (from bottom to top)
	        currentRowPointer -= unpaddedRowSize;
        }

        fclose(imageFile);
}

//***Inputs*****
//fileName: The name of the file to save 
//pixels: Pointer to the pixel data array
//colors: Pointer to the color table data array
//width: The width of the image in pixels
//height: The height of the image in pixels
//bitssPerPixel: The number of bits per pixel that are used in the image
//resx,resy: The X resolution and Y resolution of the image in pixels/meter
void WriteBMPtoICO(const char *fileName, byte *pixels,byte *colors, int32 width, int32 height,int16 bitsPerPixel,int32 resx,int32 resy)
{
        //Open file in binary mode
        FILE *outputFile = fopen(fileName, "wb");
	/******_ICONHEADER_******/
	//write reserved as 00 00
	int16 reserved=0x0000;
	fwrite(&reserved,2,1,outputFile);
	//write imageType 00 01 for .ico file
	int16 imageType=0x0001;
	fwrite(&imageType,2,1,outputFile);
	//write noOfImages(Here it is 1)
	int16 noOfImages=0x0001;
	fwrite(&noOfImages,2,1,outputFile);
	/****_ICONDIRECTORY_****/
	//write width of icon file(here it is width of img to be converted)
	byte w=width;
	fwrite(&w,1,1,outputFile);
	//write height of icon file(here it is height of img to be converted)
	byte h=height;
	fwrite(&h,1,1,outputFile);
	//write color pallete(0 if bitsPerPixel>=8)
	byte col=0x0000;
	fwrite(&col,1,1,outputFile);
	//write reserved as 00
	byte reserve=0x0000;
	fwrite(&reserve,1,1,outputFile);
	//write color planes=00 01 for .ico file
	int16 cplanes=0x0001;
	fwrite(&cplanes,2,1,outputFile);
	//write bitsPerPixel of bitmap file to be converted
	fwrite(&bitsPerPixel,2,1,outputFile);
	//write the total filesize of .ico file
	int32 fileSize=40 + pow(2,bitsPerPixel) + (w*h*2);
	fwrite(&fileSize,4,1,outputFile);
	//write the bmp data offset
	int32 bmpDataOffset=0x0016;
	fwrite(&bmpDataOffset,4,1,outputFile);
        /*******_BITMAP_INFO_HEADER_******/
        //Write size
        int32 infoHeaderSize = INFO_HEADER_SIZE;
        fwrite(&infoHeaderSize, 4, 1, outputFile);
        //Write width and height
        fwrite(&width, 4, 1, outputFile);
	int32 h2=2*height;
        fwrite(&h2, 4, 1, outputFile);
        //Write planes
        int16 planes = 1; //always 1
        fwrite(&planes, 2, 1, outputFile);
        //write bits per pixel
        fwrite(&bitsPerPixel, 2, 1, outputFile);
        //write compression
        int32 compression = NO_COMPRESION;
        fwrite(&compression, 4, 1, outputFile);
        //write image size (in bytes)
	int32 bytesPerPixel= ((int32)bitsPerPixel/8);
        int32 imageSize = width*height*bytesPerPixel;
        fwrite(&imageSize, 4, 1, outputFile);
        //write resolution (in pixels per meter)
        int32 resolutionX = resx; 
        int32 resolutionY = resy; 
        fwrite(&resolutionX, 4, 1, outputFile);
        fwrite(&resolutionY, 4, 1, outputFile);
        //write colors used 
        int32 colorsUsed = MAX_NUMBER_OF_COLORS;
        fwrite(&colorsUsed, 4, 1, outputFile);
        //Write important colors
        int32 importantColors = ALL_COLORS_REQUIRED;
        fwrite(&importantColors, 4, 1, outputFile);
	//If file is 1-bit,4-bit or 8-bit then write its color table
	if(bytesPerPixel<= 1)
	{	
		int numberOfColors= pow(2,bitsPerPixel);
		fwrite(colors, 1, 4*numberOfColors,outputFile);
	}
        //write data
        int i = 0;
	//calculate padded and unpadded row sizes(unpadded for pixel array data access and padded for writing)
        int paddedRowSize = (int)(4 * ceil((float)(width) / 4.0f))*(bytesPerPixel);
        int unpaddedRowSize = width*bytesPerPixel;
        for ( i = 0; i < height; i++)
        {
                int pixelOffset = ((height - i) - 1)*unpaddedRowSize;
                fwrite(&pixels[pixelOffset], 1, paddedRowSize, outputFile);
        }
	//if bits of image < 32, the AND mask is supplied for transparency effects
	if(bitsPerPixel < 32)
	{	byte mask=0x0000;
		for( i=0 ; i<(w*h) ;i++)
		{	
			/*if(pixels[i] == 255)
				mask=0x0000;
			else
				mask=0x00FF;*/
			fwrite(&mask,1,1,outputFile);
		}
	}
        fclose(outputFile);
}

int main()
{
        byte *pixels;
	byte *colors;
        int32 width;
        int32 height;
        int16 bitsPerPixel;
	int32 resx;
	int32 resy;
        ReadBMP("img.bmp", &pixels, &colors, &width, &height,&bitsPerPixel,&resx,&resy);
        WriteBMPtoICO("img2.ico", pixels, colors, width, height, bitsPerPixel, resx, resy);
        free(pixels);
        return 0;
}
