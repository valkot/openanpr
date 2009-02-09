#ifndef IMAGE_H_
#define IMAGE_H_

struct Image 
{
	unsigned int Width;
	unsigned int Height;
	unsigned char BytesPerPixel;
	unsigned char *Data;
};

#endif /*IMAGE_H_*/
