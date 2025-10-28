#ifndef IMAGE_H
#define IMAGE_H

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>

/**
 * @brief Entier naturel sur 24-bit
 * @param byte Tableau de 3 bytes
 */
typedef struct uint24_t
{
    uint8_t byte[3];
}uint24_t;

/**
 * @brief Triple buffer d'entier naturel sur 24-bit
 * @param bytes 3 pointeurs sur 1 byte
 */
typedef struct uint24_t_ptr
{
    uint8_t* bytes[3];
}uint24_t_ptr;

/**
 * @brief Image avec un buffer codé sur 24-bit
 * @param width Longueur de l'image
 * @param height Hauteur de l'image
 * @param img_file Pointeur sur notre fichier image
 * @param buffer Buffer 24-bit
 */
typedef struct Image_24bit
{
    size_t      width;
    size_t      height;
    FILE*       img_file;
    uint24_t*   buffer;
}Image_24bit;

/**
 * @brief Image avec un triple buffer codé sur 24-bit
 * @param width Longueur de l'image
 * @param height Hauteur de l'image
 * @param img_file Pointeur sur notre fichier image
 * @param buffer Triple Buffer 24-bit
 */
typedef struct Image_24bit_ptr
{
    size_t       width;
    size_t       height;
    FILE*        img_file;
    uint24_t_ptr buffer;
}Image_24bit_ptr;

/**
 * @brief Image avec un buffer codé sur 32-bit
 * @param width Longueur de l'image
 * @param height Hauteur de l'image
 * @param img_file Pointeur sur notre fichier image
 * @param buffer Buffer 32-bit
 */
typedef struct Image_32bit
{
    size_t      width;
    size_t      height;
    FILE*       img_file;
    uint32_t*   buffer;
}Image_32bit;

/**
 * @brief Créer une image avec une taille de buffer de 3(24 bit) * width * height
 * @param width Longueur
 * @param height Hauteur
 */
Image_24bit*        create_image_24bit(const size_t width, const size_t height);
/**
 * @brief Créer une image avec 3 buffer rgb de taille 3(24 bit) * width * height
 * @param width Longueur
 * @param height Hauteur
 */
Image_24bit_ptr*    create_image_24bit_ptr(const size_t width, const size_t height);
/**
 * @brief Créer une image avec une taille de buffer de 4(32 bit) * width * height
 * @param width Longueur
 * @param height Hauteur
 */
Image_32bit*        create_image_32bit(const size_t width, const size_t height);

/**
 * @brief
 * Parametre une couleur représenter avec un entier non signé (Entier naturel)
 * sous 32 bit, i.e 0xAARRGGBB. (AA Alpha n'est pas utilisé dans la fonction)
 * @param r La partie rouge de la couleur
 * @param g La partie verte de la couleur
 * @param b La partie bleu de la couleur
 * @return une couleur en 32 bit
 */
uint32_t get_color_32bit(const uint8_t r, const uint8_t g, const uint8_t b);


/**
 * @brief  * Parametre une couleur représenter avec un entier non signé (Entier naturel)
 * sous 24 bit, i.e 0xRRGGBB.
 * @param color Couleur sous 24 bit
 * @param r La partie rouge de la couleur
 * @param g La partie verte de la couleur
 * @param b La partie bleu de la couleur*
 */
void set_color_24bit(uint24_t* color,const uint8_t r, const uint8_t g, const uint8_t b);

/**
 * @brief  * Parametre une couleur représenter avec un entier non signé (Entier naturel)
 * sous 24 bit, i.e 0xRRGGBB au n-ième tableau.
 * @param color Couleur sous 24 bit
 * @param i i-ème couleur
 * @param r La partie rouge de la couleur
 * @param g La partie verte de la couleur
 * @param b La partie bleu de la couleur*
 */
void set_color_24bit_ptr(uint24_t_ptr* color,const size_t i,const uint8_t r, const uint8_t g, const uint8_t b);

/**
 * @brief Ecrit un pixel sur une image
 * @param img Fichier image
 * @param color Couleur 32 bit
 */
void write_pixel_color_32bit(FILE* img, const uint32_t color);

/**
 * @brief Ecrit un pixel sur une image
 * @param img Fichier image
 * @param color Couleur 24 bit
 */
void write_pixel_color_24bit(FILE* img, uint24_t* color);

/**
 * @brief Ecrit un pixel sur une image
 * @param img Fichier image
 * @param color Couleur 24 bit
 * @param i i-eme coleur
 */
void write_pixel_color_24bit_ptr(FILE* img, uint24_t_ptr* color, const size_t i);

/**
 * @brief Remplit les pixel d'une couleur
 * @param img Image à remplir
 * @param r Rouge
 * @param g Vert
 * @param b Bleu
 */
void clear_frame_color_24bit(Image_24bit *const img, const uint8_t r, const uint8_t g, const uint8_t b);

/**
 * @brief Remplit les pixel d'une couleur
 * @param img Image à remplir
 * @param r Rouge
 * @param g Vert
 * @param b Bleu
 */
void clear_frame_color_24bit_ptr(Image_24bit_ptr *const img, const uint8_t r, const uint8_t g, const uint8_t b);

/**
 * @brief Remplit les pixel d'une couleur
 * @param img Image à remplir
 * @param r Rouge
 * @param g Vert
 * @param b Bleu
 */
void clear_frame_color_32bit(Image_32bit *const img, const uint8_t r, const uint8_t g, const uint8_t b);

/**
 * @brief Remplit les pixel de couleur pour créer un effet de ciel
 * @param img Image à remplir
 */
void clear_frame_sky_color_24bit(Image_24bit *const img);

/**
 * @brief Remplit les pixel de couleur pour créer un effet de ciel
 * @param img Image à remplir
 */
void clear_frame_sky_color_24bit_ptr(Image_24bit_ptr *const img);

/**
 * @brief Remplit les pixel de couleur pour créer un effet de ciel
 * @param img Image à remplir
 */
void clear_frame_sky_color_32bit(Image_32bit *const img);

/**
 * @brief Ecrit sur le fichier image
 * @param img Image à écrire
 */
void write_image_file_24bit(Image_24bit *const img);

/**
 * @brief Ecrit sur le fichier image
 * @param img Image à écrire
 */
void write_image_file_24bit_ptr(Image_24bit_ptr *const img);

/**
 * @brief Ecrit sur le fichier image
 * @param img Image à écrire
 */
void write_image_file_32bit(Image_32bit *const img);

/**
 * @brief Libère la mémoire allouée par une image
 * @param img La mémoire allouée à liberer
 */
void free_image_24bit(Image_24bit* img);

/**
 * @brief Libère la mémoire allouée par une image
 * @param img La mémoire allouée à liberer
 */
void free_image_24bit_ptr(Image_24bit_ptr* img);

/**
 * @brief Libère la mémoire allouée par une image
 * @param img La mémoire allouée à liberer
 */
void free_image_32bit(Image_32bit* img);




#endif