/**
 * @file bdpt.c
 * @brief Implementation autonome du Bidirectional Path Tracing (Veach 1997).
 *
 * Voir bdpt.h pour la vue d'ensemble. Toutes les fonctions internes sont
 * statiques afin de garder l'API publique reduite a bdpt_render_pixel.
 *
 * Conventions internes utilisees dans tout le fichier:
 *   - Les couleurs des primitives sont deja normalisees dans [0,1] (cf.
 *     create_primitive_ext). On les utilise telles quelles.
 *   - BRDF Lambertienne : f_r = obj->color * obj->albedo / pi.
 *   - Pour la reflexion speculaire pure, BSDF * cos / pdf = albedo (delta).
 *   - PDF stockees en mesure d'AIRE (unite : 1 / m^2) sauf indication contraire.
 *   - Echantillonnage des directions par cosinus pondere, pdf_solid = cos / pi.
 */

#include "bdpt.h"

#include <math.h>
#include <float.h>
#include <stdlib.h>

/* ------------------------------------------------------------------------- */
/*                              Helpers internes                             */
/* ------------------------------------------------------------------------- */

/**
 * @brief Renvoie x si x > 0, 1 sinon. Utile pour eviter les divisions par
 *        zero dans le calcul des ratios MIS lorsqu'une PDF vaut 0 (delta).
 */
static inline float remap0(float x){
	return (x > 0.f) ? x : 1.f;
}

/**
 * @brief Genere un float uniforme dans [0,1[.
 */
static inline float rand01(unsigned int* seed){
	return (float)rand_r(seed) / (float)RAND_MAX;
}

/**
 * @brief Calcule la normale geometrique au point d'impact selon le type
 *        de primitive. Variante locale du helper utilise dans light.c afin
 *        de garder ce module independant.
 *
 * @param p          Primitive touchee.
 * @param hit        Position de l'impact.
 * @param is_intern  1 si le rayon vient de l'interieur de la primitive.
 * @param face       Indice de face renvoye par l'intersecteur (boites).
 * @param n_out      Normale geometrique sortante.
 */
static void compute_hit_normal(const Primitive* p, const Vector* hit,
                               int is_intern, int face, Vector* n_out){
	switch (p->type) {
		case SPHERE:
			*n_out = get_normal_vector_sphere(hit, &p->position);
			break;
		case BBOX:
			*n_out = get_normal_vector_box(face, is_intern);
			break;
		case BOX:
		{
			OBB* box = (OBB*)p->object;
			Vector n;
			switch (face) {
				case MIN:    n = box->obb_right;     mul_ext(&n, -1.f, &n); break;
				case MAX:    n = box->obb_right;     break;
				case BOTTOM: n = box->obb_up;        mul_ext(&n, -1.f, &n); break;
				case UP:     n = box->obb_up;        break;
				case BACK:   n = box->obb_direction; mul_ext(&n, -1.f, &n); break;
				case FRONT:  n = box->obb_direction; break;
				default:     create_vector_ext(&n, 0.f, 1.f, 0.f); break;
			}
			if (is_intern) mul_ext(&n, -1.f, &n);
			*n_out = n;
			break;
		}
	}
}

/**
 * @brief Construit un rayon offset legerement dans le sens de la normale
 *        pour eviter les auto-intersections ("shadow acne").
 */
static inline void make_offset_origin(const Vector* hit, const Vector* n, Vector* out){
	Vector eps;
	mul_ext(n, EPS, &eps);
	add_ext(hit, &eps, out);
}

/**
 * @brief Convertit une PDF en mesure d'angle solide en PDF en mesure d'aire
 *        sur la surface de "to", vue depuis "from".
 *
 * Formule : pdf_area = pdf_solid * |cos(theta_to)| / dist^2
 *
 * @param pdf_solid  PDF en sr^-1 (nul retourne 0).
 * @param from       Sommet emetteur.
 * @param to         Sommet recepteur.
 */
static float solid_to_area_pdf(float pdf_solid, const BDPT_Vertex* from, const BDPT_Vertex* to){
	if (pdf_solid <= 0.f) return 0.f;
	Vector w;
	sub_ext(&to->position, &from->position, &w);
	float d2 = dot(&w, &w);
	if (d2 < 1e-20f) return 0.f;
	float inv_d = 1.f / sqrtf(d2);
	Vector w_hat = {{ w.Data[0]*inv_d, w.Data[1]*inv_d, w.Data[2]*inv_d }};
	float cos_to = fabsf(dot(&to->normal, &w_hat));
	return pdf_solid * cos_to / d2;
}

/* ------------------------------------------------------------------------- */
/*                      BSDF : evaluation, PDF, sampling                     */
/* ------------------------------------------------------------------------- */

/**
 * @brief Evalue la BRDF f_r d'un sommet de surface dans la base (wi, wo).
 *
 * Pour une surface Lambertienne : f_r = color * albedo / pi.
 * Pour une surface Specular pure : f_r est une distribution de Dirac, la
 * fonction renvoie 0 (utiliser la sortie de bsdf_sample directement).
 *
 * @param v   Sommet de surface.
 * @param wi  Direction entrante (depuis le sommet, vers la lumiere "amont").
 * @param wo  Direction sortante (depuis le sommet, vers la lumiere "aval").
 * @param f   Sortie : valeur RGB de la BRDF.
 */
static void bsdf_eval(const BDPT_Vertex* v, const Vector* wi, const Vector* wo, Vector* f){
	(void)wi; (void)wo;
	if (!v->obj) { create_vector_default_ext(f); return; }

	if (v->obj->m_type == Lambertian || v->obj->m_type == Emissive) {
		const float k = v->obj->albedo / (float)M_PI;
		f->Data[0] = v->obj->color.Data[0] * k;
		f->Data[1] = v->obj->color.Data[1] * k;
		f->Data[2] = v->obj->color.Data[2] * k;
		return;
	}
	create_vector_default_ext(f);
}

/**
 * @brief PDF en mesure d'angle solide de l'echantillonnage de wo
 *        connaissant wi a un sommet de surface.
 *
 * @return cos(theta_o) / pi pour Lambertian ; 0 pour Specular (delta).
 */
static float bsdf_pdf_solid(const BDPT_Vertex* v, const Vector* wi, const Vector* wo){
	(void)wi;
	if (!v->obj || v->obj->m_type == Specular) return 0.f;
	float c = dot(&v->normal, wo);
	if (c <= 0.f) return 0.f;
	return c * (float)(M_1_PI);
}

/**
 * @brief Tire une nouvelle direction de propagation au sommet v selon la BRDF.
 *
 * @param v          Sommet de surface.
 * @param wi         Direction d'arrivee (vers le sommet precedent).
 * @param wo         Sortie : direction de propagation echantillonnee (unitaire).
 * @param f_times_cos Sortie : f_r * |cos(theta_o)| (deja groupe pour multiplier
 *                    le throughput sans risque de division par zero pour Specular).
 * @param pdf_solid  Sortie : PDF en angle solide (0 pour Specular).
 * @param is_delta   Sortie : 1 si l'evenement echantillonne est delta.
 * @param seed       Graine RNG.
 *
 * @return 1 si la direction est valide (cos > 0), 0 sinon.
 */
static int bsdf_sample(const BDPT_Vertex* v, const Vector* wi,
                       Vector* wo, Vector* f_times_cos, float* pdf_solid, int* is_delta,
                       unsigned int* seed){
	*is_delta = 0;
	create_vector_default_ext(f_times_cos);
	*pdf_solid = 0.f;

	if (!v->obj) return 0;

	switch (v->obj->m_type) {
		case Lambertian:
		case Emissive: {
			Vector zero = {{0.f, 0.f, 0.f}};
			Ray r = random_Ray_demi_sphere_cosine_weighted(&zero, &v->normal, seed);
			*wo = r.direction;
			float c = dot(&v->normal, wo);
			if (c <= 0.f) return 0;
			*pdf_solid = c * (float)M_1_PI;
			/* f_r * cos = (color * albedo / pi) * cos */
			float k = v->obj->albedo * c * (float)M_1_PI;
			f_times_cos->Data[0] = v->obj->color.Data[0] * k;
			f_times_cos->Data[1] = v->obj->color.Data[1] * k;
			f_times_cos->Data[2] = v->obj->color.Data[2] * k;
			return 1;
		}
		case Specular: {
			/* reflexion miroir : wo = wi - 2(wi . n) n  avec wi pointant vers la surface */
			float dn = dot(wi, &v->normal);
			Vector w;
			w.Data[0] = wi->Data[0] - 2.f * dn * v->normal.Data[0];
			w.Data[1] = wi->Data[1] - 2.f * dn * v->normal.Data[1];
			w.Data[2] = wi->Data[2] - 2.f * dn * v->normal.Data[2];
			norm_ext(&w, &w);
			*wo = w;
			*is_delta = 1;
			*pdf_solid = 0.f; /* delta */
			/* throughput *= albedo (couleur preservee, pas de teinte ajoutee) */
			f_times_cos->Data[0] = v->obj->albedo;
			f_times_cos->Data[1] = v->obj->albedo;
			f_times_cos->Data[2] = v->obj->albedo;
			return 1;
		}
	}
	return 0;
}

/* ------------------------------------------------------------------------- */
/*                           Echantillonnage des lumieres                    */
/* ------------------------------------------------------------------------- */

/**
 * @brief Choisit une lumiere proportionnellement a son albedo.
 *
 * @return Indice de la lumiere choisie, et probabilite de selection dans *p_choose.
 */
static size_t pick_light(const Scene* S, unsigned int* seed, float* p_choose){
	float total = 0.f;
	for (size_t i = 0; i < S->size_lights; ++i) total += S->lights[i]->albedo;
	if (total <= 0.f) { *p_choose = 0.f; return 0; }
	float u = rand01(seed) * total;
	float acc = 0.f;
	for (size_t i = 0; i < S->size_lights; ++i) {
		acc += S->lights[i]->albedo;
		if (u <= acc) {
			*p_choose = S->lights[i]->albedo / total;
			return i;
		}
	}
	*p_choose = S->lights[S->size_lights-1]->albedo / total;
	return S->size_lights - 1;
}

/**
 * @brief Calcule l'aire totale d'une primitive emissive (sphere ou boite).
 */
static float light_surface_area(const Primitive* L){
	switch (L->type) {
		case SPHERE: {
			Sphere* s = (Sphere*)L->object;
			return 4.f * (float)M_PI * s->radius * s->radius;
		}
		case BOX:
		case BBOX: {
			OBB* b = (OBB*)L->object;
			float ax = b->size.Data[0], ay = b->size.Data[1], az = b->size.Data[2];
			/* size = demi-extent => face XY = (2ax)(2ay) = 4 ax ay */
			return 8.f * (ax*ay + ax*az + ay*az);
		}
	}
	return 0.f;
}

/**
 * @brief Echantillonne uniformement un point sur la surface d'une lumiere
 *        et renvoie sa normale sortante ainsi que la PDF en mesure d'aire.
 *
 * Pour les boites on tire une face uniformement (1/6) puis un point uniforme
 * sur cette face. La PDF en aire vaut alors (1/6)/aire_face.
 *
 * @param L          Primitive emissive.
 * @param seed       Graine RNG.
 * @param pos        Sortie : position echantillonnee.
 * @param normal     Sortie : normale sortante a la surface.
 * @param pdf_area   Sortie : PDF de la position en mesure d'aire (1 / m^2).
 */
static void sample_light_point(const Primitive* L, unsigned int* seed,
                               Vector* pos, Vector* normal, float* pdf_area){
	switch (L->type) {
		case SPHERE: {
			Sphere* sph = (Sphere*)L->object;
			float u1 = rand01(seed);
			float u2 = rand01(seed);
			float z = 2.f*u1 - 1.f;
			float r = sqrtf(fmaxf(0.f, 1.f - z*z));
			float phi = 2.f * (float)M_PI * u2;
			Vector dir = {{ r*cosf(phi), r*sinf(phi), z }};
			pos->Data[0] = L->position.Data[0] + dir.Data[0] * sph->radius;
			pos->Data[1] = L->position.Data[1] + dir.Data[1] * sph->radius;
			pos->Data[2] = L->position.Data[2] + dir.Data[2] * sph->radius;
			*normal = dir; /* deja unitaire */
			*pdf_area = 1.f / (4.f * (float)M_PI * sph->radius * sph->radius);
			return;
		}
		case BOX:
		case BBOX: {
			OBB* b = (OBB*)L->object;
			int face = (int)(rand_r(seed) % 6);
			float u = rand01(seed) * 2.f - 1.f;
			float v = rand01(seed) * 2.f - 1.f;
			Vector axis_n, axis_u, axis_v;
			float ext_n, ext_u, ext_v;

			switch (face) {
				case 0: axis_n = b->obb_right;     ext_n = b->size.Data[0];
				        axis_u = b->obb_up;        ext_u = b->size.Data[1];
				        axis_v = b->obb_direction; ext_v = b->size.Data[2];
				        mul_ext(&axis_n, -1.f, &axis_n); break;
				case 1: axis_n = b->obb_right;     ext_n = b->size.Data[0];
				        axis_u = b->obb_up;        ext_u = b->size.Data[1];
				        axis_v = b->obb_direction; ext_v = b->size.Data[2]; break;
				case 2: axis_n = b->obb_up;        ext_n = b->size.Data[1];
				        axis_u = b->obb_right;     ext_u = b->size.Data[0];
				        axis_v = b->obb_direction; ext_v = b->size.Data[2];
				        mul_ext(&axis_n, -1.f, &axis_n); break;
				case 3: axis_n = b->obb_up;        ext_n = b->size.Data[1];
				        axis_u = b->obb_right;     ext_u = b->size.Data[0];
				        axis_v = b->obb_direction; ext_v = b->size.Data[2]; break;
				case 4: axis_n = b->obb_direction; ext_n = b->size.Data[2];
				        axis_u = b->obb_right;     ext_u = b->size.Data[0];
				        axis_v = b->obb_up;        ext_v = b->size.Data[1];
				        mul_ext(&axis_n, -1.f, &axis_n); break;
				case 5:
				default:axis_n = b->obb_direction; ext_n = b->size.Data[2];
				        axis_u = b->obb_right;     ext_u = b->size.Data[0];
				        axis_v = b->obb_up;        ext_v = b->size.Data[1]; break;
			}

			pos->Data[0] = L->position.Data[0]
			               + axis_n.Data[0]*ext_n + axis_u.Data[0]*u*ext_u + axis_v.Data[0]*v*ext_v;
			pos->Data[1] = L->position.Data[1]
			               + axis_n.Data[1]*ext_n + axis_u.Data[1]*u*ext_u + axis_v.Data[1]*v*ext_v;
			pos->Data[2] = L->position.Data[2]
			               + axis_n.Data[2]*ext_n + axis_u.Data[2]*u*ext_u + axis_v.Data[2]*v*ext_v;

			*normal = axis_n;
			float face_area = 4.f * ext_u * ext_v;
			*pdf_area = (1.f / 6.f) / face_area;
			return;
		}
	}
	create_vector_default_ext(pos);
	create_vector_ext(normal, 0.f, 1.f, 0.f);
	*pdf_area = 0.f;
}

/* ------------------------------------------------------------------------- */
/*                          Random walk (sous-chemin)                        */
/* ------------------------------------------------------------------------- */

/**
 * @brief Etend un sous-chemin par un random walk a partir du sommet path[start-1]
 *        deja initialise. Ne traite ni la generation du premier sommet ni la
 *        connexion : c'est purement un suivi de rayons avec mise a jour des
 *        throughputs et des PDF en aire (forward).
 *
 * @param tree         Acceleration BVH.
 * @param S            Scene (utilisee pour le fond ; les couleurs background ne
 *                     contribuent que via la strategie s=0 si on rate la scene).
 * @param path         Tableau pre-alloue de BDPT_Vertex (taille >= max_depth).
 * @param start        Indice du premier sommet a generer (= index du sommet a
 *                     creer ; le sommet path[start-1] doit etre valide).
 * @param max_depth    Capacite du tableau path.
 * @param ray          Rayon initial sortant de path[start-1] (direction unitaire).
 * @param beta         Throughput courant a transporter.
 * @param pdf_solid_fwd PDF en angle solide de la direction du rayon initial,
 *                     sampling au sommet path[start-1] (0 si delta).
 * @param prev_delta   1 si le sommet precedent etait delta (specular).
 * @param seed         Graine RNG.
 *
 * @return Nombre total de sommets dans le chemin (>= start).
 */
static int random_walk(Large_BVH_t* tree, const Scene* S,
                       BDPT_Vertex* path, int start, int max_depth,
                       Ray ray, Vector beta, float pdf_solid_fwd,
                       int prev_delta, unsigned int* seed){
	(void)S;
	int n = start;
	float pdfFwd = pdf_solid_fwd;
	float pdfRev = 0.f;
	int prev_was_delta = prev_delta;

	for (int bounces = 0; bounces < max_depth - start; ++bounces) {
		if (n >= max_depth) break;

		float t = FLT_MAX;
		Primitive* obj = NULL;
		int is_intern = 0, face = -1;

		if (!intersect_in_clusters(tree, &ray, &t, &obj, &is_intern, &face) || !obj) {
			return n; /* miss : sous-chemin termine */
		}

		Vector hit;
		linear_ext(&ray.position, &ray.direction, t, &hit);
		Vector ng;
		compute_hit_normal(obj, &hit, is_intern, face, &ng);
		/* Normale orientee cote rayon entrant */
		if (dot(&ng, &ray.direction) > 0.f) mul_ext(&ng, -1.f, &ng);
		norm_ext(&ng, &ng);

		BDPT_Vertex* v  = &path[n];
		BDPT_Vertex* vp = &path[n-1];

		v->v_type    = BDPT_VERTEX_SURFACE;
		v->position  = hit;
		v->normal    = ng;
		v->obj       = obj;
		v->throughput = beta;
		v->delta     = 0;
		v->is_light  = (obj->m_type == Emissive);
		v->pdf_rev   = 0.f;

		/* PDF forward en mesure d'aire : conversion solide -> aire entre vp et v.
		 * Si la direction sortante de vp etait delta, par convention la pdf
		 * forward du sommet courant vaut 0 (ne contribue pas dans MIS). */
		if (prev_was_delta) {
			v->pdf_fwd = 0.f;
		} else {
			v->pdf_fwd = solid_to_area_pdf(pdfFwd, vp, v);
		}

		++n;

		/* Si on est tombe sur une lumiere, on s'arrete : aucun rebond suivant. */
		if (obj->m_type == Emissive) {
			return n;
		}

		/* Echantillonnage de la direction sortante */
		Vector wo;
		Vector f_cos;
		float pdf_solid_out = 0.f;
		int is_delta = 0;
		Vector wi_dir = ray.direction; /* arrivee sur la surface */
		if (!bsdf_sample(v, &wi_dir, &wo, &f_cos, &pdf_solid_out, &is_delta, seed)) {
			return n;
		}
		v->delta = is_delta;

		/* Mise a jour du throughput : beta *= f_r * cos / pdf_solid.
		 * Pour Specular (delta), on a stocke directement f * cos / pdf = albedo dans f_cos. */
		if (is_delta) {
			beta.Data[0] *= f_cos.Data[0];
			beta.Data[1] *= f_cos.Data[1];
			beta.Data[2] *= f_cos.Data[2];
		} else {
			if (pdf_solid_out <= 0.f) return n;
			float inv = 1.f / pdf_solid_out;
			beta.Data[0] *= f_cos.Data[0] * inv;
			beta.Data[1] *= f_cos.Data[1] * inv;
			beta.Data[2] *= f_cos.Data[2] * inv;
		}

		/* PDF reverse au sommet vp : echantillonner la direction "wi -> vp" depuis v
		 * (utilise pour la cohesion temporelle des PDF reverse en cas de besoin MIS).
		 * Pour Lambertien, p_solid_rev = cos(angle entre n et -wi) / pi. */
		Vector neg_wi;
		mul_ext(&wi_dir, -1.f, &neg_wi);
		float pdf_solid_rev = 0.f;
		if (!is_delta) {
			float cr = dot(&v->normal, &neg_wi);
			if (cr > 0.f) pdf_solid_rev = cr * (float)M_1_PI;
		}
		pdfRev = pdf_solid_rev;
		vp->pdf_rev = solid_to_area_pdf(pdfRev, v, vp);

		/* Russian roulette apres quelques rebonds */
		int depth_so_far = n - start; /* nombre de sommets generes lors de ce walk */
		if (depth_so_far > 3) {
			float q = beta.Data[0]*0.2126f + beta.Data[1]*0.7152f + beta.Data[2]*0.0722f;
			if (q < 0.05f) q = 0.05f;
			if (q > 1.f) q = 1.f;
			if (rand01(seed) > q) return n;
			beta.Data[0] /= q; beta.Data[1] /= q; beta.Data[2] /= q;
		}

		/* Prochain rayon */
		Vector orig;
		make_offset_origin(&hit, &v->normal, &orig);
		ray.position = orig;
		ray.direction = wo;
		pdfFwd = pdf_solid_out;
		prev_was_delta = is_delta;
	}
	return n;
}

/* ------------------------------------------------------------------------- */
/*                       Generation des sous-chemins                         */
/* ------------------------------------------------------------------------- */

/**
 * @brief Construit le sous-chemin camera (oeil + max_depth-1 rebonds).
 *
 * @return Nombre de sommets stockes (>= 1).
 */
static int generate_camera_subpath(int x, int y, const Scene* S, Large_BVH_t* tree,
                                   BDPT_Vertex* path, int max_depth, unsigned int* seed){
	if (max_depth < 1) return 0;

	BDPT_Vertex* c0 = &path[0];
	c0->v_type    = BDPT_VERTEX_CAMERA;
	c0->position  = S->camera.position;
	c0->normal    = S->camera.direction;
	create_vector_ext(&c0->throughput, 1.f, 1.f, 1.f);
	c0->obj       = NULL;
	c0->pdf_fwd   = 1.f;
	c0->pdf_rev   = 0.f;
	c0->delta     = 1;     /* camera pinhole : position deterministe */
	c0->is_light  = 0;

	if (max_depth < 2) return 1;

	Ray r;
	trace_ray((size_t)x, (size_t)y, &S->camera, &r);

	Vector beta = {{1.f, 1.f, 1.f}};
	/* Pour pinhole, on traite c[1].pdf_fwd comme une constante (= 1) :
	 * cette constante est partagee par toutes les strategies (s, t>=2)
	 * et s'annule donc dans les ratios MIS. */
	return random_walk(tree, S, path, 1, max_depth, r, beta, 1.f, /*prev_delta=*/1, seed);
}

/**
 * @brief Construit le sous-chemin lumiere a partir d'un point echantillonne
 *        sur la surface d'une primitive emissive choisie aleatoirement.
 *
 * Le sommet 0 contient :
 *   - position et normale sur la surface de la lumiere
 *   - throughput beta_0 = Le / pdf_pos_area
 *   - pdf_fwd = pdf_pos_area (en aire)
 *
 * Le sommet 1 (premiere intersection) est genere par random walk en
 * echantillonnant la direction sortante par hemisphere cosinus.
 *
 * @return Nombre de sommets stockes (0 si pas de lumieres dans la scene).
 */
static int generate_light_subpath(const Scene* S, Large_BVH_t* tree,
                                  BDPT_Vertex* path, int max_depth, unsigned int* seed){
	if (max_depth < 1 || S->size_lights == 0) return 0;

	float p_choose = 0.f;
	size_t li = pick_light(S, seed, &p_choose);
	if (p_choose <= 0.f) return 0;
	Primitive* L = S->lights[li];

	Vector pos, n_light;
	float pdf_pos_area = 0.f;
	sample_light_point(L, seed, &pos, &n_light, &pdf_pos_area);
	if (pdf_pos_area <= 0.f) return 0;
	float pdf_y0 = p_choose * pdf_pos_area;

	BDPT_Vertex* l0 = &path[0];
	l0->v_type   = BDPT_VERTEX_LIGHT;
	l0->position = pos;
	l0->normal   = n_light;
	l0->obj      = L;
	l0->pdf_fwd  = pdf_y0;
	l0->pdf_rev  = 0.f;
	l0->delta    = 0;
	l0->is_light = 1;

	/* Le radiance emis par une surface diffuse : Le_radiance = albedo * color
	 * (convention du projet : color en [0,1], albedo > 1 pour les emissifs).
	 * beta_0 = Le / pdf_y0. */
	Vector Le = { { L->color.Data[0]*L->albedo,
	                L->color.Data[1]*L->albedo,
	                L->color.Data[2]*L->albedo } };
	mul_ext(&Le, 1.f / pdf_y0, &l0->throughput);

	if (max_depth < 2) return 1;

	/* Direction initiale : hemisphere cosinus oriente selon n_light */
	Ray r = random_Ray_demi_sphere_cosine_weighted(&pos, &n_light, seed);
	float cos_l = dot(&n_light, &r.direction);
	if (cos_l <= 0.f) return 1;
	float pdf_solid_dir = cos_l * (float)M_1_PI;

	/* beta apres l'emission : beta_0 * Le_dir * cos / pdf_dir
	 * Pour un emetteur diffus, Le_dir = 1 / pi (deja inclus dans la convention
	 * "color * albedo" qui represente l'integrale d'emission ; on garde donc
	 * juste le facteur cos/pdf_dir = pi/cos * cos = pi).
	 * Concretement : facteur multiplicatif = M_PI (annule le 1/pi de Le_dir). */
	Vector beta = l0->throughput;
	float k = (float)M_PI; /* = cos / pdf_solid_dir, vu que pdf=cos/pi */
	(void)cos_l;
	beta.Data[0] *= k;
	beta.Data[1] *= k;
	beta.Data[2] *= k;

	/* Offset pour eviter l'auto-intersection sur la surface emettrice */
	Vector orig;
	make_offset_origin(&pos, &n_light, &orig);
	r.position = orig;

	return random_walk(tree, S, path, 1, max_depth, r, beta, pdf_solid_dir, /*prev_delta=*/0, seed);
}

/* ------------------------------------------------------------------------- */
/*                              Visibilite                                   */
/* ------------------------------------------------------------------------- */

/**
 * @brief Test de visibilite entre deux points en ray-castant a travers la scene.
 *
 * @param tree    Acceleration BVH.
 * @param a       Premier point (rayon offset selon n_a).
 * @param n_a     Normale en a (pour offset).
 * @param b       Second point.
 * @return 1 si a et b se voient mutuellement (rien d'opaque entre eux), 0 sinon.
 */
static int visible(Large_BVH_t* tree, const Vector* a, const Vector* n_a, const Vector* b){
	Vector dir;
	sub_ext(b, a, &dir);
	float d = length(&dir);
	if (d <= 1e-5f) return 0;
	float inv = 1.f / d;
	dir.Data[0] *= inv; dir.Data[1] *= inv; dir.Data[2] *= inv;

	Ray r;
	make_offset_origin(a, n_a, &r.position);
	r.direction = dir;

	float t = FLT_MAX;
	Primitive* obj = NULL;
	int is_intern = 0, face = -1;
	if (!intersect_in_clusters(tree, &r, &t, &obj, &is_intern, &face)) return 1;
	/* Visible si l'intersection est strictement plus loin que la cible.
	 * On tolere une marge pour les sommets situes EXACTEMENT sur b
	 * (cas s>=1 : l'intersection trouvee EST le sommet lumineux). */
	return (t >= d - 10.f * EPS);
}

/* ------------------------------------------------------------------------- */
/*                           Calcul du poids MIS                             */
/* ------------------------------------------------------------------------- */

/**
 * @brief Calcule le poids MIS (power heuristic, beta=2) de la strategie (s, t)
 *        en redefinissant temporairement les PDF aux sommets de connexion.
 *
 * Methode des ratios de PBRT : on parcourt les sous-chemins en cumulant
 * r_i = pdf_rev / pdf_fwd, et on somme les r_i^2 pour les indices ou la
 * contribution est non delta. Le poids final est 1 / (1 + sum_ri^2).
 *
 * @param camera     Sous-chemin camera.
 * @param t          Nombre de sommets camera utilises (>= 2).
 * @param light      Sous-chemin lumiere.
 * @param s          Nombre de sommets lumiere utilises (>= 0).
 * @param S          Scene (pour parametres d'echantillonnage des lumieres).
 */
static float mis_weight(BDPT_Vertex* camera, int t,
                        BDPT_Vertex* light,  int s,
                        const Scene* S){
	if (s + t == 2) return 1.f;

	/* Sauvegarde / restauration des PDF qui vont etre redefinies localement. */
	BDPT_Vertex* pt      = (t > 0) ? &camera[t-1] : NULL;
	BDPT_Vertex* qs      = (s > 0) ? &light[s-1]  : NULL;
	BDPT_Vertex* pt_minus= (t > 1) ? &camera[t-2] : NULL;
	BDPT_Vertex* qs_minus= (s > 1) ? &light[s-2]  : NULL;

	float save_pt_pdfRev      = pt       ? pt->pdf_rev      : 0.f;
	float save_pt_minus_pdfRev= pt_minus ? pt_minus->pdf_rev: 0.f;
	float save_qs_pdfRev      = qs       ? qs->pdf_rev      : 0.f;
	float save_qs_minus_pdfRev= qs_minus ? qs_minus->pdf_rev: 0.f;
	int   save_pt_delta       = pt       ? pt->delta        : 0;
	int   save_qs_delta       = qs       ? qs->delta        : 0;

	if (pt) pt->delta = 0;
	if (qs) qs->delta = 0;

	/* --- Recalcul des PDF reverse aux sommets de connexion --- */

	/* pt->pdf_rev : probabilite (en aire) de generer pt depuis le cote lumiere. */
	if (pt) {
		if (s == 0) {
			/* Strategie s=0 : pt est sur la lumiere (ou rien a faire). On
			 * fixe pt->pdf_rev au pdf de positionner pt comme point lumineux. */
			float p_choose = 0.f;
			float total = 0.f;
			for (size_t i = 0; i < S->size_lights; ++i) total += S->lights[i]->albedo;
			if (pt->obj && pt->is_light && total > 0.f) {
				p_choose = pt->obj->albedo / total;
				float A = light_surface_area(pt->obj);
				pt->pdf_rev = (A > 0.f) ? (p_choose / A) : 0.f;
			} else {
				pt->pdf_rev = 0.f;
			}
		} else if (qs) {
			/* Direction qs -> pt en mesure solide selon BSDF a qs (ou
			 * direction d'emission au sommet lumineux si s == 1). */
			Vector d;
			sub_ext(&pt->position, &qs->position, &d);
			float dist = length(&d);
			if (dist > 0.f) {
				d.Data[0]/=dist; d.Data[1]/=dist; d.Data[2]/=dist;
				float pdf_solid;
				if (s == 1) {
					/* Emission : cos/pi sur l'hemisphere cote normale. */
					float c = dot(&qs->normal, &d);
					pdf_solid = (c > 0.f) ? c * (float)M_1_PI : 0.f;
				} else {
					Vector wi_at_qs;
					sub_ext(&light[s-2].position, &qs->position, &wi_at_qs);
					norm_ext(&wi_at_qs, &wi_at_qs);
					pdf_solid = bsdf_pdf_solid(qs, &wi_at_qs, &d);
				}
				pt->pdf_rev = solid_to_area_pdf(pdf_solid, qs, pt);
			} else {
				pt->pdf_rev = 0.f;
			}
		}
	}

	/* pt_minus->pdf_rev : probabilite de generer pt_minus depuis pt en venant du cote lumiere. */
	if (pt_minus && pt) {
		Vector wo_at_pt;
		if (s > 0 && qs) {
			sub_ext(&qs->position, &pt->position, &wo_at_pt);
		} else {
			/* s == 0 : direction vers "rien" cote lumiere ; on prend une
			 * direction arbitraire reliant pt a un point sur sa propre normale.
			 * En pratique pour s=0 + length>=3, pt_minus->pdf_rev est utilisee
			 * via l'emission diffuse cos/pi vers pt_minus. */
			sub_ext(&pt_minus->position, &pt->position, &wo_at_pt);
			mul_ext(&wo_at_pt, -1.f, &wo_at_pt); /* pour qu'on aille "depuis pt vers pt_minus" */
		}
		norm_ext(&wo_at_pt, &wo_at_pt);

		Vector wi_at_pt;
		sub_ext(&pt_minus->position, &pt->position, &wi_at_pt);
		norm_ext(&wi_at_pt, &wi_at_pt);

		float pdf_solid;
		if (s == 0) {
			/* emission diffuse */
			float c = dot(&pt->normal, &wi_at_pt);
			pdf_solid = (c > 0.f) ? c * (float)M_1_PI : 0.f;
		} else {
			pdf_solid = bsdf_pdf_solid(pt, &wo_at_pt, &wi_at_pt);
		}
		pt_minus->pdf_rev = solid_to_area_pdf(pdf_solid, pt, pt_minus);
	}

	/* qs->pdf_rev : probabilite de generer qs depuis pt en venant du cote camera. */
	if (qs && pt) {
		Vector wi_at_pt;
		if (t > 1 && pt_minus) sub_ext(&pt_minus->position, &pt->position, &wi_at_pt);
		else                    create_vector_default_ext(&wi_at_pt);
		norm_ext(&wi_at_pt, &wi_at_pt);

		Vector wo_at_pt;
		sub_ext(&qs->position, &pt->position, &wo_at_pt);
		norm_ext(&wo_at_pt, &wo_at_pt);

		float pdf_solid = bsdf_pdf_solid(pt, &wi_at_pt, &wo_at_pt);
		qs->pdf_rev = solid_to_area_pdf(pdf_solid, pt, qs);
	}

	/* qs_minus->pdf_rev : probabilite de generer qs_minus depuis qs cote camera. */
	if (qs_minus && qs && pt) {
		Vector wi_at_qs;
		sub_ext(&pt->position, &qs->position, &wi_at_qs);
		norm_ext(&wi_at_qs, &wi_at_qs);

		Vector wo_at_qs;
		sub_ext(&qs_minus->position, &qs->position, &wo_at_qs);
		norm_ext(&wo_at_qs, &wo_at_qs);

		float pdf_solid = bsdf_pdf_solid(qs, &wi_at_qs, &wo_at_qs);
		qs_minus->pdf_rev = solid_to_area_pdf(pdf_solid, qs, qs_minus);
	}

	/* --- Sommation des ratios r_i^2 --- */
	float sumRi = 0.f;

	float ri = 1.f;
	for (int i = t - 1; i > 0; --i) {
		ri *= remap0(camera[i].pdf_rev) / remap0(camera[i].pdf_fwd);
		int delta_im1 = camera[i-1].delta;
		if (!camera[i].delta && !delta_im1) sumRi += ri * ri;
	}

	ri = 1.f;
	for (int i = s - 1; i >= 0; --i) {
		ri *= remap0(light[i].pdf_rev) / remap0(light[i].pdf_fwd);
		int delta_im1;
		if (i > 0) delta_im1 = light[i-1].delta;
		else       delta_im1 = 0; /* lumiere de surface = pas delta */
		if (!light[i].delta && !delta_im1) sumRi += ri * ri;
	}

	/* Restauration */
	if (pt)       { pt->pdf_rev       = save_pt_pdfRev;       pt->delta = save_pt_delta; }
	if (pt_minus) { pt_minus->pdf_rev = save_pt_minus_pdfRev; }
	if (qs)       { qs->pdf_rev       = save_qs_pdfRev;       qs->delta = save_qs_delta; }
	if (qs_minus) { qs_minus->pdf_rev = save_qs_minus_pdfRev; }

	return 1.f / (1.f + sumRi);
}

/* ------------------------------------------------------------------------- */
/*                                Connexion                                  */
/* ------------------------------------------------------------------------- */

/**
 * @brief Calcule la contribution non-ponderee d'une strategie (s, t).
 *
 * Pour s == 0 : contribution = beta_camera_t-1 * Le emise au sommet camera
 *               (utile uniquement si camera[t-1] est sur une surface emissive).
 * Pour s >= 1 : contribution = beta_c * f_c * G * V * f_l * beta_l, ou
 *               beta_l est le throughput du sommet lumiere et f_l la BRDF
 *               associee (pour s == 1, f_l est implicitement integree dans
 *               l'expression Le et beta_l contient deja Le/pdf_pos).
 *
 * @return Contribution couleur (radiance) avant ponderation MIS.
 */
static Vector connect_paths(const Scene* S, Large_BVH_t* tree,
                            BDPT_Vertex* camera, int t,
                            BDPT_Vertex* light,  int s){
	(void)S;
	Vector L = {{0.f, 0.f, 0.f}};
	if (t < 2) return L;

	BDPT_Vertex* ct = &camera[t-1];

	if (s == 0) {
		if (!ct->is_light || !ct->obj) return L;
		Vector Le = { { ct->obj->color.Data[0] * ct->obj->albedo,
		                ct->obj->color.Data[1] * ct->obj->albedo,
		                ct->obj->color.Data[2] * ct->obj->albedo } };
		L.Data[0] = ct->throughput.Data[0] * Le.Data[0];
		L.Data[1] = ct->throughput.Data[1] * Le.Data[1];
		L.Data[2] = ct->throughput.Data[2] * Le.Data[2];
		return L;
	}

	if (s < 1) return L;
	BDPT_Vertex* ls = &light[s-1];

	/* On ne connecte pas via des sommets delta. */
	if (ct->delta || ls->delta) return L;
	if (!ct->obj) return L;

	/* Direction et facteur geometrique */
	Vector d;
	sub_ext(&ls->position, &ct->position, &d);
	float dist2 = dot(&d, &d);
	if (dist2 < 1e-12f) return L;
	float dist = sqrtf(dist2);
	float inv = 1.f / dist;
	Vector wo_c = { { d.Data[0]*inv, d.Data[1]*inv, d.Data[2]*inv } };
	Vector wo_l = { { -wo_c.Data[0], -wo_c.Data[1], -wo_c.Data[2] } };

	float cos_c = dot(&ct->normal, &wo_c);
	float cos_l = dot(&ls->normal, &wo_l);
	if (cos_c <= 0.f || cos_l <= 0.f) return L;
	float G = cos_c * cos_l / dist2;

	/* BSDF a chaque extremite */
	Vector wi_c;
	if (t > 1) sub_ext(&camera[t-2].position, &ct->position, &wi_c);
	else       create_vector_ext(&wi_c, 0.f, 0.f, 1.f);
	norm_ext(&wi_c, &wi_c);
	Vector f_c;
	bsdf_eval(ct, &wi_c, &wo_c, &f_c);
	if (f_c.Data[0] + f_c.Data[1] + f_c.Data[2] <= 0.f) return L;

	Vector f_l;
	if (s == 1) {
		/* Le sommet lumiere n'a pas de BRDF reflechissante. La contribution
		 * de l'emission (Le) est deja incluse dans light[0].throughput
		 * (pre-divisee par pdf_pos_area). On utilise donc f_l = 1 en
		 * preservant le cos cote lumiere via G. */
		create_vector_ext(&f_l, 1.f, 1.f, 1.f);
	} else {
		Vector wi_l;
		sub_ext(&light[s-2].position, &ls->position, &wi_l);
		norm_ext(&wi_l, &wi_l);
		bsdf_eval(ls, &wi_l, &wo_l, &f_l);
		if (f_l.Data[0] + f_l.Data[1] + f_l.Data[2] <= 0.f) return L;
	}

	/* Test de visibilite */
	if (!visible(tree, &ct->position, &ct->normal, &ls->position)) return L;

	L.Data[0] = ct->throughput.Data[0] * f_c.Data[0] * G * f_l.Data[0] * ls->throughput.Data[0];
	L.Data[1] = ct->throughput.Data[1] * f_c.Data[1] * G * f_l.Data[1] * ls->throughput.Data[1];
	L.Data[2] = ct->throughput.Data[2] * f_c.Data[2] * G * f_l.Data[2] * ls->throughput.Data[2];

	return L;
}

/* ------------------------------------------------------------------------- */
/*                              Entry point                                  */
/* ------------------------------------------------------------------------- */

void bdpt_render_pixel(int x, int y,
                       const Scene* S,
                       Large_BVH_t* tree,
                       int max_depth,
                       Vector* pixel_color,
                       unsigned int* seed){
	if (max_depth < 2) max_depth = 2;

	BDPT_Vertex camera_path[max_depth];
	BDPT_Vertex light_path[max_depth];

	/* Init defensive (les champs critiques sont reecris par les generateurs). */
	for (int i = 0; i < max_depth; ++i) {
		camera_path[i].pdf_fwd = 0.f; camera_path[i].pdf_rev = 0.f;
		camera_path[i].delta   = 0;   camera_path[i].is_light = 0;
		camera_path[i].obj     = NULL;
		light_path[i].pdf_fwd  = 0.f; light_path[i].pdf_rev  = 0.f;
		light_path[i].delta    = 0;   light_path[i].is_light = 0;
		light_path[i].obj      = NULL;
	}

	int nC = generate_camera_subpath(x, y, S, tree, camera_path, max_depth, seed);
	int nL = generate_light_subpath(S, tree, light_path, max_depth, seed);

	Vector L = {{0.f, 0.f, 0.f}};

	/* Cas : le rayon camera a manque toute la scene -> contribution background.
	 * On l'ajoute uniquement pour le rayon primaire (t == 1) afin d'eviter
	 * un double comptage avec les autres strategies. */
	if (nC == 1 && S->background_color) {
		L.Data[0] += S->background_color->Data[0];
		L.Data[1] += S->background_color->Data[1];
		L.Data[2] += S->background_color->Data[2];
	}

	/* Boucle sur toutes les strategies (s, t) avec t >= 2. */
	for (int t = 2; t <= nC; ++t) {
		for (int s = 0; s <= nL; ++s) {
			int depth = s + t - 2; /* nombre de segments */
			if (depth < 1) continue;

			Vector C = connect_paths(S, tree, camera_path, t, light_path, s);
			if (C.Data[0] == 0.f && C.Data[1] == 0.f && C.Data[2] == 0.f) continue;

			float w = mis_weight(camera_path, t, light_path, s, S);
			L.Data[0] += w * C.Data[0];
			L.Data[1] += w * C.Data[1];
			L.Data[2] += w * C.Data[2];
		}
	}

	pixel_color->Data[0] += L.Data[0];
	pixel_color->Data[1] += L.Data[1];
	pixel_color->Data[2] += L.Data[2];
}
