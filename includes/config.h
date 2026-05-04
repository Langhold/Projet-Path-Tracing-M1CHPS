#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "light.h"
#include <simd/fLight.h>

typedef enum
{
  russian,
  naive,
  SIMD,
  trees,
  cluster,
  bdpt
} Implem_t;

struct Scene;

/// @brief Configuration of the problem to solve.
typedef struct pt_config_t
{
  /// Number of samples.
  size_t samples;
  /// Number of .
  size_t print_rate;
  /// Width of the simulation.
  int width;
  /// Height of the simulation.
  int height;
  /// Reynolds number.
  int bounces;
  /// Derived flow parameter.
  int n_measures;
  /// Output file.
  const char *output_filename;
  const char *output_measures;
  /// Kind of benchmark.
  void (*benchmark)(struct Scene *, size_t, size_t);

  /// For simd
  void (*vbenchmark)(fScene *, size_t, size_t);

  /// Output file.
  const char *benchmark_name;

  Implem_t implem;
  const char *implem_name;
  /// If user want only final image.
  int can_print;
} pt_config_t;

/// Configuration accessible as a global variable.
/// To be used as a constant unless for the initial load.

/// @brief Load the configuration from a file.
/// @param filename Path of the config file.
void load_config(pt_config_t *config, const char *filename);

/// @brief Pretty-print of the configuration.
void print_config(pt_config_t *config);

/// @brief Default values in case the user did not define everything in the config file.
void setup_default_values(pt_config_t *config);
