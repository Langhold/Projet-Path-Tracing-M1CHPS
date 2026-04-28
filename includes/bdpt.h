#ifndef BDPT_H
#define BDPT_H

/**
 * @file bdpt.h
 * @brief Bidirectional Path Tracing (Veach 1997) self-contained module.
 *
 * Cette implementation repose uniquement sur les structures et utilitaires
 * du projet (Scene, Primitive, Camera, Vector, Ray, Large_BVH_t,
 * intersect_in_clusters, random_Ray_demi_sphere_cosine_weighted, trace_ray)
 * et est independante des fonctions BDPT historiques de light.c.
 *
 * Vue d'ensemble de l'algorithme:
 *   1. Pour chaque pixel on construit deux sous-chemins:
 *        - un sous-chemin camera c[0..nC-1] partant de l'oeil
 *        - un sous-chemin lumiere l[0..nL-1] partant d'une surface emissive
 *   2. Pour chaque couple d'indices (s, t), avec s sommets cote lumiere
 *      et t sommets cote camera, on tente de relier l[s-1] et c[t-1] par
 *      un rayon d'ombre, on evalue la contribution puis on applique un
 *      poids MIS (power heuristic, beta = 2) calcule a partir des PDF
 *      forward et reverse en mesure d'aire stockees a chaque sommet.
 *   3. Toutes les contributions ponderees sont accumulees dans la
 *      radiance du pixel.
 *
 * Strategies couvertes:
 *   - s = 0, t >= 2 : un rayon camera a frappe directement une lumiere
 *     (equivaut au path tracing naif).
 *   - s >= 1, t >= 2 : connexion explicite d'un sommet camera a un
 *     sommet lumiere via test de visibilite.
 *
 * La strategie t = 1 (light tracing avec splat sur l'image) n'est pas
 * implementee ici (camera pinhole ponctuelle); les autres strategies
 * restent non biaisees grace a la renormalisation MIS sur les seules
 * strategies effectivement evaluees.
 *
 * Conventions:
 *   - Couleurs internes deja normalisees dans [0,1] (cf. create_primitive_ext).
 *   - BRDF Lambertienne f_r = color * albedo / pi.
 *   - Echantillonnage cosinus pondere sur l'hemisphere : pdf_solid = cos / pi.
 *   - Materiau Specular traite comme delta de Dirac (pas de connexion).
 */

#include "scene.h"
#include "ray.h"
#include "vector.h"

/** Type semantique d'un sommet d'un sous-chemin BDPT. */
typedef enum BDPT_VertexType{
	BDPT_VERTEX_CAMERA,   /**< Sommet origine du sous-chemin camera (oeil). */
	BDPT_VERTEX_LIGHT,    /**< Sommet origine du sous-chemin lumiere (sur surface emissive). */
	BDPT_VERTEX_SURFACE   /**< Sommet de surface generique (Lambertien ou Specular). */
} BDPT_VertexType;

/**
 * @brief Sommet d'un sous-chemin BDPT.
 *
 * On conserve les PDF en mesure d'aire afin de pouvoir calculer le
 * poids MIS de toutes les strategies (s, t) sans avoir a re-tracer
 * de rayons.
 */
typedef struct BDPT_Vertex{
	BDPT_VertexType v_type;  /**< Type semantique du sommet. */

	Vector position;         /**< Position 3D du sommet. */
	Vector normal;           /**< Normale geometrique sortante au sommet (orientee cote rayon entrant). */
	Vector throughput;       /**< Throughput cumule beta_i, transporte AU sommet i sans inclure
	                              la BRDF a evaluer pour la prochaine direction. */

	Primitive* obj;          /**< Primitive touchee (NULL pour le sommet camera). */

	float pdf_fwd;           /**< PDF en mesure d'aire de generer ce sommet depuis le precedent
	                              dans le sens de propagation du sous-chemin. */
	float pdf_rev;           /**< PDF en mesure d'aire de generer ce sommet depuis le suivant
	                              (sens inverse du sous-chemin). Initialise a 0 et mis a jour
	                              localement lors du calcul MIS. */

	int delta;               /**< Vrai si le sommet a ete echantillonne via une distribution de
	                              Dirac (ex. reflexion speculaire) : aucune connexion possible. */
	int is_light;            /**< Vrai si le sommet repose sur une surface emissive. */
} BDPT_Vertex;

/**
 * @brief Calcule la radiance d'un pixel par BDPT.
 *
 * Cette fonction est destinee a etre appelee depuis la boucle principale
 * de rendu (ppm.c) en remplacement de path_trace_t. Elle est thread-safe
 * tant que chaque thread possede sa propre graine RNG (parametre seed).
 *
 * @param x            Coordonnee X du pixel (0..width-1).
 * @param y            Coordonnee Y du pixel (0..height-1).
 * @param S            Scene contenant les objets, lumieres et la camera.
 * @param tree         Structure d'acceleration (clusters BVH) pour les intersections.
 * @param max_depth    Profondeur maximale de chaque sous-chemin (>= 2 conseille).
 * @param pixel_color  En sortie : la radiance estimee est ajoutee (additive) a *pixel_color.
 * @param seed         Graine RNG mutable (rand_r-compatible) propre au thread appelant.
 */
void bdpt_render_pixel(int x, int y,
                       const Scene* S,
                       Large_BVH_t* tree,
                       int max_depth,
                       Vector* pixel_color,
                       unsigned int* seed);

#endif /* BDPT_H */
