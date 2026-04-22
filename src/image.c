#include "image.h"
#include "vector.h"

Image_32bit* create_image_32bit(const size_t width, const size_t height)
{
    Image_32bit* img = (Image_32bit*)malloc(sizeof(Image_32bit));
    if(img == NULL)
    {
        fprintf(stderr,"Failed to allocate memory.\n");
        exit(1);
    }
    
    img->width = width;
    img->height = height;
	

    img->buffer = (uint32_t*)malloc(width * height * sizeof(uint32_t));

    if(img->buffer == NULL)
    {
        fprintf(stderr, "Error : Failed to allocate memory buffer.\n");
        exit(1);  
    }

    memset(img->buffer, 0, width * height * sizeof(uint32_t));

    return img;
};


uint32_t get_color_32bit(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
{
    return a << 24 | r << 16 | g << 8 | b;
}

void put_color_at_32bit(Image_32bit* const img, const size_t x, const size_t y, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
{
   img->buffer[y * img->width + x] = get_color_32bit(r,g,b,a);
}

void write_pixel_color_32bit(FILE* img, const uint32_t color)
{
    fprintf(img, "%d %d %d", (color & 0xFF0000) >> 16, (color & 0x00FF00) >> 8, (color & 0x0000FF));
}

void clear_frame_color_32bit(Image_32bit *const img, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a)
{
    const uint32_t color = get_color_32bit(r, g, b, a);
    memset(img->buffer, color, img->height * img->width * sizeof(uint32_t));
}

void write_image_file_32bit(Image_32bit *const img, const size_t current_samples)
{
	if (mpi_rank != 0) return;

	char path[100];
	sprintf(path, "image/image_32bit%ld.ppm", current_samples);
	
	FILE *file = fopen(path, "w");
	
	if(file == NULL) {
	 fprintf(stderr, "Error: Failed to create an image file.\n");
	 exit(1);
	}
	
	fprintf(file, "P3\n%ld %ld\n%d\n", img->width, img->height, 255);
	fflush(file);

    for(size_t i = 0; i < img->height * img->width; ++i)
    {
        write_pixel_color_32bit(file, img->buffer[i]);        
        fprintf(file, "\n");
    }
	fclose(file);
}

void free_image_32bit(Image_32bit* img)
{
    if(img)
    {
        if(img->buffer) free(img->buffer);
        else fprintf(stderr, "Image buffer already freed.\n");

        free(img);
    }
    else
    {
        fprintf(stderr, "Image already freed.\n");
    }
    
}
