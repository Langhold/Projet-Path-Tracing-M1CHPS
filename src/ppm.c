#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

typedef struct uint24_t
{
    uint8_t byte[3];
}uint24_t;

typedef struct uint24_t_ptr
{
    uint8_t* bytes[3];
}uint24_t_ptr;

/**
 * @brief
 * Parametre une couleur représenter avec un entier non signé (Entier naturel)
 * sous 32 bit, i.e 0xAARRGGBB. (AA Alpha n'est pas utilisé dans la fonction)
 * @skip  
 * Exemple:
 * 
 *          On veut la couleur jaune
 * 
 *          set_pixel_color(255, 255, 0)
 * 
 *          255 => 1111 1111
 * 
 *          0   => 0000 0000
 * 
 *          RESULAT de TAILLE 32 bit => 0000 0000 0000 0000 0000 0000 0000 0000
 * 
 *          On effectue un shift de 16 bit vers la gauche pour la partie rouge :
 * 
 *                   1111 1111  << 16 =   1111 1111 0000 0000 0000 0000
 * 
 *              + un ET binaire :
 * 
 *                   1111 1111 0000 0000 0000 0000 | 0000 0000 0000 0000 0000 0000 0000 0000 = 0000 0000 1111 1111 0000 0000 0000 0000
 * 
 *         Ensuite, un shift de 8 bit vers la gauche pour la partie verte :
 * 
 *                   1111 1111  << 8 =   1111 1111 0000 0000    
 *   
 *              + un ET binaire :
 * 
 *                   1111 1111 0000 0000 | 0000 0000 1111 1111 0000 0000 0000 0000 = 0000 0000 1111 1111 1111 1111 0000 0000 
 * 
 *         Enfin, un ET binaire pour la partie bleu :
 * 
 *                   0000 0000 | 0000 0000 1111 1111 1111 1111 0000 0000 = 0000 0000 1111 1111 1111 1111 0000 0000
 * 
 *         Donc notre couleur jaune vaut 0000 0000 1111 1111 1111 1111 0000 0000 soit 0x00FFFF00 => 16776960
 * 
 * @param r La partie rouge de la couleur
 * @param g La partie verte de la couleur
 * @param b La partie bleu de la couleur
 * @return une couleur en 32 bit
 */
uint32_t get_color_32bit(const uint8_t r, const uint8_t g, const uint8_t b)
{
    return r << 16 | g << 8 | b;
}


/**
 * @brief  * Parametre une couleur représenter avec un entier non signé (Entier naturel)
 * sous 24 bit, i.e 0xRRGGBB.
 * @param color Couleur sous 24 bit
 * @param r La partie rouge de la couleur
 * @param g La partie verte de la couleur
 * @param b La partie bleu de la couleur*
 */
void set_color_24bit(uint24_t* color,const uint8_t r, const uint8_t g, const uint8_t b)
{
    color->byte[0] = r;
    color->byte[1] = g;
    color->byte[2] = b;
}

/**
 * @brief  * Parametre une couleur représenter avec un entier non signé (Entier naturel)
 * sous 24 bit, i.e 0xRRGGBB au n-ième tableau.
 * @param color Couleur sous 24 bit
 * @param i i-ème couleur
 * @param r La partie rouge de la couleur
 * @param g La partie verte de la couleur
 * @param b La partie bleu de la couleur*
 */
void set_color_24bit_ptr(uint24_t_ptr* color,const size_t i,const uint8_t r, const uint8_t g, const uint8_t b)
{
    color->bytes[0][i] = r;
    color->bytes[1][i] = g;
    color->bytes[2][i] = b;
}

/**
 * @brief Ecrit un pixel sur une image
 * @param img Fichier image
 * @param color Couleur 32 bit
 */
void write_pixel_color_32bit(FILE* img, const uint32_t color)
{
    fprintf(img, "%d ", (color & 0xFF0000) >> 16);
    fprintf(img, "%d ", (color & 0x00FF00) >> 8);
    fprintf(img, "%d ", (color & 0x0000FF));
}

/**
 * @brief Ecrit un pixel sur une image
 * @param img Fichier image
 * @param color Couleur 24 bit
 */
void write_pixel_color_24bit(FILE* img, uint24_t* color)
{
    fprintf(img, "%d ", color->byte[0]);
    fprintf(img, "%d ", color->byte[1]);
    fprintf(img, "%d ", color->byte[2]);
}

/**
 * @brief Ecrit un pixel sur une image
 * @param img Fichier image
 * @param color Couleur 24 bit
 * @param i i-eme coleur
 */
void write_pixel_color_24bit_ptr(FILE* img, uint24_t_ptr* color, const size_t i)
{
    fprintf(img, "%d ", color->bytes[0][i]);
    fprintf(img, "%d ", color->bytes[1][i]);
    fprintf(img, "%d ", color->bytes[2][i]);
}

int main(int argc, char** argv)
{

    const size_t height = 800;
    const size_t width  = 600;

    if(strcmp(argv[1], "32") == 0)
    {
        //Les pixels sur l'image final sont codés sur 24 bit et non 32 bit(taille uniquement utilisée pour le buffer)
        FILE* img = fopen("image/img_32bit.ppm", "w");

        if(img == NULL)
        {
            fprintf(stderr, "Erreur : echec de la creation du fichier\n");
            exit(0);
        }

        uint32_t* buffer = (uint32_t*)malloc(height * width * sizeof(uint32_t));

        if(buffer == NULL)
        {
            fprintf(stderr, "Erreur : echec de l'allocation de la memoire\n");
            exit(0);
        }

        for(size_t i = 0; i < height; ++i)
        {
            for(size_t j = 0; j < width; ++j)
            {
                // Génere un effet de ciel bleu selon la hauteur de l'image 
                const uint8_t k = i * 255 / height;
                buffer[i * width + j] = get_color_32bit( k % 255, k % 255, 255);
            }
        }

        // Ecrit les paramètre indispensable pour le format de l'image
        //P3
        //height width
        //255
        fprintf(img, "P3\n%ld %ld\n%d\n", height, width, 255);

        for(size_t i = 0; i < height * width; ++i)
        {
            write_pixel_color_32bit(img, buffer[i]);        
            fprintf(img, "\n");
        }

        fclose(img);

        free(buffer);
    }
    else if(strcmp(argv[1], "24") == 0)
    {
        FILE* img = fopen("image/img_24bit.ppm", "w");

        if(img == NULL)
        {
            fprintf(stderr, "Erreur : echec de la creation du fichier\n");
            exit(0);
        }
        
        uint24_t* buffer = (uint24_t*)malloc(height * width * sizeof(uint24_t));

        if(buffer == NULL)
        {
            fprintf(stderr, "Erreur : echec de l'allocation de la memoire\n");
            exit(0);
        }

        for(size_t i = 0; i < height; ++i)
        {
            for(size_t j = 0; j < width; ++j)
            {
                // Génere un effet de ciel bleu selon la hauteur de l'image 
                const uint8_t k = i * 255 / height;
                set_color_24bit(&buffer[i * width + j], k % 255, k % 255, 255);
            }
        }

        // Ecrit les paramètre indispensable pour le format de l'image
        //P3
        //height width
        //255
        fprintf(img, "P3\n%ld %ld\n%d\n", height, width, 255);

        for(size_t i = 0; i < height * width; ++i)
        {
            write_pixel_color_24bit(img, &buffer[i]);        
            fprintf(img, "\n");
        }

        fclose(img);

        free(buffer);
    }
    else if(strcmp(argv[1], "24ptr") == 0)
    {
        FILE* img = fopen("image/img_24bit_ptr.ppm", "w");

        if(img == NULL)
        {
            fprintf(stderr, "Erreur : echec de la creation du fichier\n");
            exit(0);
        }
        
        uint24_t_ptr buffer;
        buffer.bytes[0] = (uint8_t*)malloc(height * width * sizeof(uint8_t));
        buffer.bytes[1] = (uint8_t*)malloc(height * width * sizeof(uint8_t));
        buffer.bytes[2] = (uint8_t*)malloc(height * width * sizeof(uint8_t));

        if(buffer.bytes[0] == NULL || buffer.bytes[1] == NULL || buffer.bytes[2] == NULL)
        {
            fprintf(stderr, "Erreur : echec de l'allocation de la memoire\n");
            exit(0);
        }

        for(size_t i = 0; i < height; ++i)
        {
            for(size_t j = 0; j < width; ++j)
            {
                // Génere un effet de ciel bleu selon la hauteur de l'image 
                const uint8_t k = i * 255 / height;
                const size_t index = i * width + j;
                set_color_24bit_ptr(&buffer, index, k % 255, k % 255, 255);
            }
        }

        // Ecrit les paramètre indispensable pour le format de l'image
        //P3
        //height width
        //255
        fprintf(img, "P3\n%ld %ld\n%d\n", height, width, 255);

        for(size_t i = 0; i < height * width; ++i)
        {
            write_pixel_color_24bit_ptr(img, &buffer, i);        
            fprintf(img, "\n");
        }

        fclose(img);
        free(buffer.bytes[0]);
        free(buffer.bytes[1]);
        free(buffer.bytes[2]);

    }
    else
    {
        fprintf(stderr, "Erreur : argument incorect");
    }

    return 0;
}