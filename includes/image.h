#ifndef IMAGE_H
#define IMAGE_H


#include "vector.h"


#define RED 0x0F00
#define GRN 0x00F0
#define BLU 0x000F

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * @brief Image avec un buffer codé sur 32-bit
 * @var width Longueur de l'image
 * @var height Hauteur de l'image
 * @var img_file Pointeur sur notre fichier image
 * @var buffer Buffer 32-bit
 */
typedef struct Image_32bit
{
    size_t      width;
    size_t      height;
    uint32_t*   buffer;
}Image_32bit;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/**
 * @brief Créer une image avec une taille de buffer de 4(32 bit) * width * height
 * @param width Longueur
 * @param height Hauteur
 */
Image_32bit* create_image_32bit(const size_t width, const size_t height);

/**
 * @brief
 * Parametre une couleur représenter avec un entier non signé (Entier naturel)
 * sous 32 bit, i.e 0xAARRGGBB. (AA Alpha n'est pas utilisé dans la fonction)
 * @param r La partie rouge de la couleur
 * @param g La partie verte de la couleur
 * @param b La partie bleu de la couleur
 * @param a La partie Alpha 
 * @return une couleur en 32 bit
 */
uint32_t get_color_32bit(const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a);

/**
 * @brief Change la couleur d'un pixel de coordonnée x,y
 * @param img Image
 * @param x X
 * @param y Y
 * @param r La partie rouge de la couleur
 * @param g La partie verte de la couleur
 * @param b La partie bleu de la couleur
 * @param a La partie Alpha
 */
void put_color_at_32bit(Image_32bit* const img, const size_t x, const size_t y, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a);

/**
 * @brief Ecrit un pixel sur une image
 * @param img Fichier image
 * @param color Couleur 32 bit
 */
void write_pixel_color_32bit(FILE* img, const uint32_t color);


/**
 * @brief Ecrit un pixel sur une image
 * @param img Fichier image
 */
void write_image_file_32bit(Image_32bit *const img, const size_t current_samples);

/**
 * @brief Remplit les pixel d'une couleur
 * @param img Image à remplir
 * @param r Rouge
 * @param g Vert
 * @param b Bleu
 * @param a Alpha
 */
void clear_frame_color_32bit(Image_32bit *const img, const uint8_t r, const uint8_t g, const uint8_t b, const uint8_t a);

/**
 * @brief Libère la mémoire allouée par une image
 * @param img La mémoire allouée à liberer
 */
void free_image_32bit(Image_32bit* img);




#endif
