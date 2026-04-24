#include <simd/fImage.h>
#include <simd/fLight.h>
#include <simd/fScene.h>
#include <simd/fRay.h>
#include <time.h>

int main(int argc, char **argv)
{
    srand(time(NULL));

    (void)argc;
    (void)argv;
    init_global_variable();

    uint64_t w = 800;
    uint64_t h = 600;

    fImage img;
    set_fImage(&img, w, h);

    fScene scene;
    fBenchmark_mouse(&scene, 800, 600);
    fpath_tracing(&scene, 100, 5, 800, 600, &img);

    free_fScene(&scene);
    create_file(&img);
    free_fImage(&img);
}