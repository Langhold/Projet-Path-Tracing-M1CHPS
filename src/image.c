#include "image.h"

Image_24bit* create_image_24bit(const size_t width, const size_t height)
{
    Image_24bit* img = (Image_24bit*)malloc(width * height * sizeof(Image_24bit));

    if(img == NULL)
    {
        fprintf(stderr,"Failed to allocate memory.\n");
        exit(1);
    }
    
    img->width = width;
    img->height = height;

    img->img_file = fopen("image/image_24bit.ppm", "w");

    if(img->img_file == NULL)
    {
        fprintf(stderr, "Error : Failed to create an image file.\n");
        exit(1);
    }

    img->buffer = (uint24_t*)malloc(width * height * sizeof(uint24_t));

    if(img->buffer == NULL)
    {
        fprintf(stderr, "Error : Failed to allocate memory buffer.\n");
        exit(1);  
    }

    memset(img->buffer, 0, width * height);

    return img;
};

Image_24bit_ptr* create_image_24bit_ptr(const size_t width, const size_t height)
{
    Image_24bit_ptr* img = (Image_24bit_ptr*)malloc(width * height * sizeof(Image_24bit_ptr));
    
    if(img == NULL)
    {
        fprintf(stderr,"Failed to allocate memory.\n");
        exit(1);
    }

    img->width = width;
    img->height = height;

    img->img_file = fopen("image/image_24bit_ptr.ppm", "w");

    if(img->img_file == NULL)
    {
        fprintf(stderr, "Error : Failed to create an image file.\n");
        exit(1);
    }


    for(size_t i = 0; i < 3; ++i)
    {
        img->buffer.bytes[i] = (uint8_t*)malloc(width * height * sizeof(uint8_t));

        if(img->buffer.bytes[i] == NULL)
        {
            fprintf(stderr, "Error : Failed to allocate memory buffer n°%ld.\n", i);
            exit(1);  
        }
        memset(img->buffer.bytes[i], 0, width * height);
    }

    return img;

};

Image_32bit* create_image_32bit(const size_t width, const size_t height)
{
    Image_32bit* img = (Image_32bit*)malloc(width * height * sizeof(Image_32bit));
    if(img == NULL)
    {
        fprintf(stderr,"Failed to allocate memory.\n");
        exit(1);
    }
    
    img->width = width;
    img->height = height;

    img->img_file = fopen("image/image_32bit.ppm", "w");

    if(img->img_file == NULL)
    {
        fprintf(stderr, "Error : Failed to create an image file.\n");
        exit(1);
    }

    img->buffer = (uint32_t*)malloc(width * height * sizeof(uint32_t));

    if(img->buffer == NULL)
    {
        fprintf(stderr, "Error : Failed to allocate memory buffer.\n");
        exit(1);  
    }

    memset(img->buffer, 0, width * height * sizeof(uint32_t));

    return img;
};


uint32_t get_color_32bit(const uint8_t r, const uint8_t g, const uint8_t b)
{
    return r << 16 | g << 8 | b;
}

void set_color_24bit(uint24_t* color,const uint8_t r, const uint8_t g, const uint8_t b)
{
    color->byte[0] = r;
    color->byte[1] = g;
    color->byte[2] = b;
}

void set_color_24bit_ptr(uint24_t_ptr* color,const size_t i,const uint8_t r, const uint8_t g, const uint8_t b)
{
    color->bytes[0][i] = r;
    color->bytes[1][i] = g;
    color->bytes[2][i] = b;
}

void write_pixel_color_24bit_ptr(FILE* img, uint24_t_ptr* color, const size_t i)
{
    fprintf(img, "%d ", color->bytes[0][i]);
    fprintf(img, "%d ", color->bytes[1][i]);
    fprintf(img, "%d ", color->bytes[2][i]);
}

void write_pixel_color_24bit(FILE* img, uint24_t* color)
{
    fprintf(img, "%d ", color->byte[0]);
    fprintf(img, "%d ", color->byte[1]);
    fprintf(img, "%d ", color->byte[2]);
}

void write_pixel_color_32bit(FILE* img, const uint32_t color)
{
    fprintf(img, "%d ", (color & 0xFF0000) >> 16);
    fprintf(img, "%d ", (color & 0x00FF00) >> 8);
    fprintf(img, "%d ", (color & 0x0000FF));
}

void clear_frame_color_24bit(Image_24bit *const img, const uint8_t r, const uint8_t g, const uint8_t b)
{
    for(size_t i = 0; i < img->height * img->width; ++i)
    {
        img->buffer[i].byte[0] = r;
        img->buffer[i].byte[1] = g;
        img->buffer[i].byte[2] = b;
    }
}

void clear_frame_color_24bit_ptr(Image_24bit_ptr *const img, const uint8_t r, const uint8_t g, const uint8_t b)
{
    for(size_t i = 0; i < img->height * img->width; ++i)
    {
        img->buffer.bytes[0][i] = r;
        img->buffer.bytes[1][i] = g;
        img->buffer.bytes[2][i] = b;
    }
}

void clear_frame_color_32bit(Image_32bit *const img, const uint8_t r, const uint8_t g, const uint8_t b)
{
    const uint32_t color = get_color_32bit(r, g, b);
    memset(img->buffer, color, img->height * img->width * sizeof(uint32_t));
}

void clear_frame_sky_color_24bit(Image_24bit *const img)
{
    for(size_t i = 0; i < img->height; ++i)
    {
        for(size_t j = 0; j < img->width; ++j)
        {
            // Génere un effet de ciel bleu selon la hauteur de l'image 
            const uint8_t k = i * 255 / img->height;
            set_color_24bit(&img->buffer[i * img->width + j], k % 255, k % 255, 255);
        }
    }
}

void clear_frame_sky_color_24bit_ptr(Image_24bit_ptr *const img)
{
    for(size_t i = 0; i < img->height; ++i)
    {
        for(size_t j = 0; j < img->width; ++j)
        {
            // Génere un effet de ciel bleu selon la hauteur de l'image 
            const uint8_t k = i * 255 / img->height;
            const size_t index = i * img->width + j;
            set_color_24bit_ptr(&img->buffer, index, k % 255, k % 255, 255);
        }
    }
}

void clear_frame_sky_color_32bit(Image_32bit *const img)
{
    for(size_t i = 0; i < img->height; ++i)
    {
        for(size_t j = 0; j < img->width; ++j)
        {
            // Génere un effet de ciel bleu selon la hauteur de l'image 
            const uint8_t k = i * 255 / img->height;
            img->buffer[i * img->width + j] = get_color_32bit( k % 255, k % 255, 255);
        }
    }   
}

void write_image_file_24bit(Image_24bit *const img)
{
    fprintf(img->img_file, "P3\n%ld %ld\n%d\n", img->height, img->width, 255);

    for(size_t i = 0; i < img->height * img->width; ++i)
    {
        write_pixel_color_24bit(img->img_file, &img->buffer[i]);        
        fprintf(img->img_file, "\n");
    }
}

void write_image_file_24bit_ptr(Image_24bit_ptr *const img)
{
    fprintf(img->img_file, "P3\n%ld %ld\n%d\n", img->height, img->width, 255);

    for(size_t i = 0; i < img->height * img->width; ++i)
    {
        write_pixel_color_24bit_ptr(img->img_file, &img->buffer, i);        
        fprintf(img->img_file, "\n");
    }
}

void write_image_file_32bit(Image_32bit *const img)
{
    fprintf(img->img_file, "P3\n%ld %ld\n%d\n", img->height, img->width, 255);

    for(size_t i = 0; i < img->height * img->width; ++i)
    {
        write_pixel_color_32bit(img->img_file, img->buffer[i]);        
        fprintf(img->img_file, "\n");
    }
}

void free_image_24bit(Image_24bit* img)
{
    if(img)
    {
        if(img->buffer) free(img->buffer);
        else fprintf(stderr, "Image buffer already freed.\n"); 

        if(img->img_file) fclose(img->img_file);
        else fprintf(stderr, "Image file already closed.\n");

        free(img);
    }
    else
    {
        fprintf(stderr, "Image already freed.\n");
    }
}

void free_image_24bit_ptr(Image_24bit_ptr* img)
{
    if(img)
    {
        for(size_t i = 0; i < 3; ++i)
        {
            if(img->buffer.bytes[i]) free(img->buffer.bytes[i]);
            else fprintf(stderr, "Image buffer n°%ld already freed.\n", i); 
        }

        if(img->img_file) fclose(img->img_file);
        else fprintf(stderr, "Image file already closed.\n");

        free(img);
    }
    else
    {
        fprintf(stderr, "Image already freed.\n");
    }
    
}

void free_image_32bit(Image_32bit* img)
{
    if(img)
    {
        if(img->buffer) free(img->buffer);
        else fprintf(stderr, "Image buffer already freed.\n"); 

        if(img->img_file) fclose(img->img_file);
        else fprintf(stderr, "Image file already closed.\n");

        free(img);
    }
    else
    {
        fprintf(stderr, "Image already freed.\n");
    }
    
}