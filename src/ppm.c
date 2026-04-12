#include "light.h"
#include <time.h>
#include <unistd.h>

#include "mpi.h"

void simul_work(const int x1, const int y1, const int local_y, const int width, Scene const * S, const size_t bounces, float* color_buffer, unsigned int* seed){
	usleep(((rand_r(seed) % 3) + 1)*1e-2);
	size_t index = (local_y * width + x1) * 3;
	color_buffer[index+0] += rand_r(seed) / (float)RAND_MAX;
	color_buffer[index+1] += rand_r(seed) / (float)RAND_MAX;
	color_buffer[index+2] += rand_r(seed) / (float)RAND_MAX;
}
static inline void color_float_to_int(float* const local_color_buffer, const int idx_rgb, uint32_t* local_pixels_buffer, const int idx, float const inv_samples){
	
	float r = local_color_buffer[idx_rgb]   * inv_samples;
	float g = local_color_buffer[idx_rgb+1] * inv_samples;
	float b = local_color_buffer[idx_rgb+2] * inv_samples;
	
	r = max(0.f, min(255.f, r));
	g = max(0.f, min(255.f, g));
	b = max(0.f, min(255.f, b));
	
	local_pixels_buffer[idx] = get_color_32bit(r, g, b, 0);
}

static inline void print_time(struct timespec const* t0, struct timespec* t1, size_t const i, size_t const smpls, size_t const bounces, const int print_rate, const int mpi_size)
{
	
	clock_gettime(CLOCK_MONOTONIC, t1);
	double elapsed = (t1->tv_sec - t0->tv_sec) + (t1->tv_nsec - t0->tv_nsec) * 1e-9;
	char* output_path = getenv("PT_MEASURES_PATH");
	if(!output_path) output_path = "runtime_by_samplings";
	char path[256];
	snprintf(path, sizeof(path), "performance/measures/%s.csv", output_path);

	bool exists = (access(path, F_OK) == 0);
	FILE* f = fopen(path, "a");
	if (!f) {
		perror("fopen");
		exit(1);
	}

	if (!exists) {
		fprintf(f, "MPI,OMP,nsamples,bounces");
		for (size_t s = print_rate; s <= smpls; s+=print_rate) {
			fprintf(f, ",%zu", s);
		}
		fprintf(f, "\n");
	}

	char* omp_num_threads = getenv("OMP_NUM_THREADS");
	int threads = omp_num_threads ? atoi(omp_num_threads) : 1;

	static bool first = 1;
	if (first) {
		fprintf(f, "%d,%d,%ld,%ld", mpi_size, threads, smpls, bounces);
		first = 0;
	}

	fprintf(f, ",%.6f", elapsed);

	if (i == smpls) {
		fprintf(f, "\n");
		first = 1;
	}

	fclose(f);
}

int main(int argc, char** argv)
{
	setvbuf(stdout, NULL, _IONBF, 0);
	
	srand((unsigned int)time(NULL));
	if(argc < 4)
	{
		fprintf(stderr,"Error : Incomplete arguments.\n Please using: %s width height samples\n",argv[0]);
		exit(1);
	}
	
	

	const int width  = atoi(argv[1]);
	const int height = atoi(argv[2]);
	const size_t smpls = atoi(argv[3]);

	size_t limit = 1;
	int can_print_image = 1;
	int measures = 1;

	if (argc > 4 && argv[4][0] != '-') {
		limit = atoi(argv[4]);
	}

	for (int arg = 4; arg < argc; ++arg) {
		if (!strcmp(argv[arg], "no_image")) {
			can_print_image = 0;
		} else if (!strcmp(argv[arg], "measures")) {
			measures = 10;
		}
	}

	for (int arg = 4; arg < argc; ++arg) {
		if (strcmp(argv[arg], "no_image") && strcmp(argv[arg], "measures") && !(arg == 4 && argv[4][0] != '-')) {
			printf("Error: please use \"no_image\" or \"measures\" instead of \"%s\".\n", argv[arg]);
			exit(1);
		}
	}
	
	
	size_t bounces = (size_t) get_bounces();

	
//######################### Create the entier scene ###########################
	Scene scene;
	benchmark_big(&scene, width, height);
	
	object_tree_t* tree = initialize_root_tree_v2(&scene);
	unsigned int seed = (unsigned int) time(NULL);
	const int K = 4;
	Large_BVH_t* tree_clusters = initialize_tree_clustering(&scene, &seed, K);
//############################################################################
	
	if(!limit) exit(1);
	size_t print_rate = smpls / limit;
	if(print_rate == 0) print_rate = 1;
	
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &mpi_size);
	

	const int per_t_height = height / mpi_size;
	float *local_color_buffer = calloc(width * per_t_height * 3, sizeof(float));
	uint32_t *local_pixels_buffer = malloc(width * per_t_height * sizeof(uint32_t));
	Image_32bit *image = create_image_32bit(width, height, smpls);
	
	
	
	for (int m = 0; m<measures; ++m) {
		memset(local_color_buffer, 0, width * per_t_height * 3* sizeof(float));
		struct timespec t0, t1;
		if (mpi_rank == 0) {
			fprintf(stdout,"Using path tracing image %dx%d.\n", width, height);
			
			clock_gettime(CLOCK_MONOTONIC, &t0);
		}
		
		int start = per_t_height*mpi_rank;
		int end = per_t_height*(mpi_rank+1);
		
	#pragma omp parallel
		{
			unsigned int seed_per_threads = time(NULL) ^ (mpi_rank << 8) ^ omp_get_thread_num();
			for(size_t i = 1; i <= smpls; ++i) {
	#pragma omp for schedule(static)
				for(int y1 = start; y1 < end; ++y1){
					for(int x1 = 0; x1 < width; ++x1){
						int local_y = y1 - per_t_height * mpi_rank;
						//path_trace(x1, y1, local_y, width, &scene, bounces, local_color_buffer, &seed_per_threads);
						path_trace_t(x1, y1, local_y, width, &scene, bounces, local_color_buffer, &seed_per_threads, tree);
						//path_trace_clusters(x1, y1, local_y, width, &scene, bounces, local_color_buffer, &seed_per_threads, tree_clusters);
					}
				}
	#pragma omp barrier
	#pragma omp single
				if (i % print_rate == 0) {
		
					if (mpi_rank == 0) clock_gettime(CLOCK_MONOTONIC, &t1);
					
					float inv_samples = 255.f / (float)i;
					
					for(int y = 0; y < per_t_height; ++y) {
						for(int x = 0; x < width; ++x) {
							int idx = y * width + x;
							int idx_rgb = idx * 3;
							color_float_to_int(local_color_buffer, idx_rgb, local_pixels_buffer, idx, inv_samples);
							
						}
					}
					
					if (mpi_size != 1){
						if (mpi_rank == 0) {
							print_time(&t0, &t1, i, smpls, bounces, print_rate, mpi_size);
							if((can_print_image)||(i==smpls)){
								MPI_Gather(local_pixels_buffer, width * per_t_height, MPI_INT32_T, image->buffer, width * per_t_height, MPI_INT32_T, 0, MPI_COMM_WORLD);
								write_image_file_32bit(image, i);
							}
							
						}
						else {
							if(can_print_image || i==smpls)
								MPI_Gather(local_pixels_buffer, width * per_t_height, MPI_INT32_T, NULL, width * per_t_height, MPI_INT32_T, 0, MPI_COMM_WORLD);
						
						}
					}
					else {
						print_time(&t0, &t1, i, smpls, bounces, print_rate, mpi_size);
						memcpy(image->buffer, local_pixels_buffer, width * per_t_height * sizeof(uint32_t));
						write_image_file_32bit(image, i);
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
