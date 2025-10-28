#include "image.h"

int main(int argc, char** argv)
{


    if(argc != 4)
    {
        fprintf(stderr,"Error : Icomplete arguments.\n");
        exit(1);
    }


    const size_t width  = atoi(argv[2]);
    const size_t height = atoi(argv[3]);

    if(strcmp(argv[1], "32") == 0)
    {
        fprintf(stdout,"Using 32-bit image %ldx%ld.\n",width,height);
        Image_32bit* image = create_image_32bit(width, height);

        clear_frame_sky_color_32bit(image);

        write_image_file_32bit(image);

        free_image_32bit(image);
    }
    else if(strcmp(argv[1], "24") == 0)
    {
        fprintf(stdout,"Using 24-bit image %ldx%ld.\n",width,height);

        Image_24bit* image = create_image_24bit(width, height);

        clear_frame_sky_color_24bit(image);

        write_image_file_24bit(image);

        free_image_24bit(image);
    }
    else if(strcmp(argv[1], "24ptr") == 0)
    {
        fprintf(stdout,"Using 24-bit ptr image %ldx%ld.\n",width,height);

        Image_24bit_ptr* image = create_image_24bit_ptr(width, height);

        clear_frame_sky_color_24bit_ptr(image);

        write_image_file_24bit_ptr(image);

        free_image_24bit_ptr(image);
    }
    else
    {
        fprintf(stderr, "Erreur : argument incorect");
    }

    return 0;
}