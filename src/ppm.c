#include "light.h"
#include "config.h"
#include "bdpt.h"
#include <time.h>
#include <unistd.h>

#include "mpi.h"

static inline void color_float_to_int(float* const local_color_buffer, const int idx_rgb, uint32_t* local_pixels_buffer, const int idx, float const inv_samples){
	static float gamma_inv = 1.f / 2.2f;
	
	float r = local_color_buffer[idx_rgb]   * inv_samples;
	float g = local_color_buffer[idx_rgb+1] * inv_samples;
	float b = local_color_buffer[idx_rgb+2] * inv_samples;
	//r = r / (1.f + r);  g = g / (1.f + g);  b = b / (1.f + b);
	/*r = powf(r, gamma_inv);*/
	/*g = powf(g, gamma_inv);*/
	/*b = powf(b, gamma_inv);*/
	if (r > 255.f) r = 255.f;
	if (g > 255.f) g = 255.f;
	if (b > 255.f) b = 255.f;

	local_pixels_buffer[idx] = get_color_32bit(r, g, b, 0);
}

static inline void print_time(struct timespec const* t0, struct timespec* t1, size_t const i, pt_config_t* config, const int mpi_size)
{
	
	static double elapsed =  0;
	elapsed += (t1->tv_sec - t0->tv_sec) + (t1->tv_nsec - t0->tv_nsec) * 1e-9;
	/*char* output_path = getenv("PT_MEASURES_PATH");
	if(!output_path) output_path = "runtime_by_samplings";
	char path[256];
	snprintf(path, sizeof(path), "performance/measures/%s.csv", output_path);*/
	
	const char* path = config->output_filename;

	bool exists = (access(path, F_OK) == 0);
	FILE* f = fopen(path, "a");
	if (!f) {
		perror("fopen");
		exit(1);
	}

	if (!exists) {
		fprintf(f, "MPI,OMP,nsamples,bounces");
		for (size_t s = config->print_rate; s <= config->samples; s+=config->print_rate) {
			fprintf(f, ",%zu", s);
		}
		fprintf(f, "\n");
	}

	char* omp_num_threads = getenv("OMP_NUM_THREADS");
	int threads = omp_num_threads ? atoi(omp_num_threads) : 1;

	static bool first = 1;
	if (first) {
		fprintf(f, "%d,%d,%zu,%d", mpi_size, threads, config->samples, config->bounces);
		first = 0;
	}

	fprintf(f, ",%.6f", elapsed);

	if (i >= config->samples) {
		fprintf(f, "\n");
		first = 1;
		elapsed = 0;
	}

	fclose(f);
}

int main(int argc, char** argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	
	srand((unsigned int)time(NULL));
	if(argc == 0)
	{
		fprintf(stderr,"Error : Incomplete arguments.\n Please using: %s <CONFIG>.txt\n",argv[0]);
		exit(1);
	}
	

	

/* ######################### SIMULATION CONFIGURATION ########################### */
	const char* path = argv[1];
	pt_config_t config;
	load_config(&config, path);
	
	const int width  = config.width;
	const int height = config.height;
	const size_t smpls = config.samples;
	const int bounces = config.bounces;
	const int measures = config.n_measures ? config.n_measures : 1;
	
	int can_print_image = config.can_print;
	size_t print_rate = config.print_rate ? config.print_rate : 1;
	
//######################### Create the entier scene ###########################
	Scene scene;
	config.benchmark(&scene, width, height);
	
	
	object_tree_t* tree = initialize_root_tree_v2(&scene);
	unsigned int seed = (unsigned int) time(NULL);
	const int K = 4;
	Large_BVH_t* tree_clusters = initialize_tree_clustering(&scene, &seed, K);
//############################################################################
	
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
	
	Image_32bit *image = create_image_32bit(width, height, smpls);
	
	struct timespec t0, t1;
	if (mpi_rank == 0) {
		print_config(&config);
		
	}

	const int per_t_height = height / mpi_size;
	float *local_color_buffer = malloc(width * per_t_height * 3* sizeof(float));
	uint32_t *local_pixels_buffer = calloc(width * per_t_height, sizeof(uint32_t));
#pragma omp parallel for schedule(static)
			for (int i = 0; i < width * per_t_height * 3; i++)
				local_color_buffer[i] = 0.0f;
	
	int start = per_t_height*mpi_rank;
	int end = per_t_height*(mpi_rank+1);
	
	
	for (int m = 0; m<measures; ++m) {
		if (mpi_rank == 0){
			printf("%d measure on %d\n", m+1, measures);
			clock_gettime(CLOCK_MONOTONIC, &t0);
		}
#pragma omp parallel
		{
			Vector pixel_color;
			unsigned int seed_per_threads = time(NULL) ^ getpid() ^ omp_get_thread_num();
			for(size_t p = print_rate; p <= smpls; p+=print_rate) {
#pragma omp for schedule(static)
				for(int y1 = start; y1 < end; ++y1){
					int local_y = y1 - (per_t_height * mpi_rank);
					for(int x1 = 0; x1 < width; ++x1){
						pixel_color.Data[0] = 0.0f;
						pixel_color.Data[1] = 0.0f;
						pixel_color.Data[2] = 0.0f;
						for (size_t i=0; i<print_rate; ++i) {
							//path_trace(x1, y1, local_y, width, &scene, bounces, local_color_buffer, &seed_per_threads);
							path_trace_t(x1, y1, &scene, bounces, &pixel_color, &seed_per_threads, tree_clusters, 0);
							//bdpt_render_pixel(x1, y1, &scene, tree_clusters, bounces, &pixel_color, &seed);
						}
						local_color_buffer[(local_y * width + x1) * 3 + 0] += pixel_color.Data[0];
						local_color_buffer[(local_y * width + x1) * 3 + 1] += pixel_color.Data[1];
						local_color_buffer[(local_y * width + x1) * 3 + 2] += pixel_color.Data[2];
					}
				}
#pragma omp single
				{
					if (mpi_rank == 0) clock_gettime(CLOCK_MONOTONIC, &t1);
					float inv_samples = 255.f / (float)p;
					
					for(int y = 0; y < per_t_height; ++y) {
						for(int x = 0; x < width; ++x) {
							int idx = y * width + x;
							int idx_rgb = idx * 3;
							color_float_to_int(local_color_buffer, idx_rgb, local_pixels_buffer, idx, inv_samples);
						}
					}
					
					if (mpi_size != 1){
						if (mpi_rank == 0) {
							print_time(&t0, &t1, p, &config, mpi_size);
							if((can_print_image)||(p==smpls)){
								MPI_Gather(local_pixels_buffer, width * per_t_height, MPI_INT32_T, image->buffer, width * per_t_height, MPI_INT32_T, 0, MPI_COMM_WORLD);
								write_image_file_32bit(image, p);
								printf("image_32bit%zu written\n", p);
							}
							
							clock_gettime(CLOCK_MONOTONIC, &t0);
						}
						else {
							if(can_print_image || p==smpls)
								MPI_Gather(local_pixels_buffer, width * per_t_height, MPI_INT32_T, NULL, width * per_t_height, MPI_INT32_T, 0, MPI_COMM_WORLD);
						}
					}
					else {
						print_time(&t0, &t1, 0, &config, mpi_size);
						memcpy(image->buffer, local_pixels_buffer, width * per_t_height * sizeof(uint32_t));
						write_image_file_32bit(image, p);
						clock_gettime(CLOCK_MONOTONIC, &t0);
					}
				}
			}
		}
	}
	MPI_Finalize();
	
	free(local_pixels_buffer); free(local_color_buffer);
	free_image_32bit(image);
	free_scene_objects(&scene); free_tree_objects(&tree);
	free_clusters(&tree_clusters);
	
	return 0;
}
