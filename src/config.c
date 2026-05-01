#include "config.h"

static const char *default_output_measures = "performance/measures/measures.csv";
static const char *default_output_filename = "image/Image";

void setup_default_values(pt_config_t *config)
{
	// Simulation parameters
	config->samples = 1000;
	config->width = 800;
	config->height = 600;
	config->output_filename = default_output_filename;
	config->output_measures = default_output_measures;
	config->n_measures = 1;
	config->bounces = 26;
	config->benchmark = &benchmark1;
	config->benchmark_name = "mickey";
	config->implem = cluster;
	config->implem_name = "cluster";
	config->can_print = 0;
	config->print_rate = 1;
}

void load_config(pt_config_t *config, const char *filename)
{
	// Vars
	FILE *fp;
	char buffer[1024];
	char buffer2[1024];
	int intValue;
	size_t size_tValue;
	int line = 0;

	// Open the config file
	fp = fopen(filename, "r");
	if (fp == NULL)
	{
		perror(filename);
		abort();
	}

	// Load default values
	setup_default_values(config);

	// Loop on lines
	while (fgets(buffer, 1024, fp) != NULL)
	{
		line++;
		if (buffer[0] == '#' || buffer[0] == '\n')
		{
			// Comment, nothing to do
		}
		else if (sscanf(buffer, "samples = %zu\n", &size_tValue) == 1)
		{
			config->samples = size_tValue;
		}
		else if (sscanf(buffer, "print rate = %zu\n", &size_tValue) == 1)
		{
			config->print_rate = size_tValue;
		}
		else if (sscanf(buffer, "width = %d\n", &intValue) == 1)
		{
			config->width = intValue;
		}
		else if (sscanf(buffer, "height = %d\n", &intValue) == 1)
		{
			config->height = intValue;
		}
		else if (sscanf(buffer, "bounces = %d\n", &intValue) == 1)
		{
			config->bounces = intValue;
		}
		else if (sscanf(buffer, "only last image = %d\n", &intValue) == 1)
		{
			config->can_print = intValue;
		}
		else if (sscanf(buffer, "n_measures = %d\n", &intValue) == 1)
		{
			config->n_measures = intValue;
		}
		else if (sscanf(buffer, "output_filename = %s\n", buffer2) == 1)
		{
			config->output_filename = strdup(buffer2);
		}
		else if (sscanf(buffer, "output_measures = %s\n", buffer2) == 1)
		{
			config->output_measures = strdup(buffer2);
		}
		else if (sscanf(buffer, "benchmark = %s\n", buffer2) == 1)
		{
			if (strcmp(buffer2, "medium") == 0)
			{
				config->benchmark = &benchmark_medium;
				config->benchmark_name = "medium";
			}
			else if (strcmp(buffer2, "huge") == 0)
			{
				config->benchmark = &benchmark_huge;
				config->benchmark_name = "huge";
			}
			else if (strcmp(buffer2, "big") == 0)
			{
				config->benchmark = &benchmark_big;
				config->benchmark_name = "big";
			}
			else
			{
				config->benchmark = &benchmark1;
				config->benchmark_name = "mickey";
			}
		}
		else if (sscanf(buffer, "vbenchmark = %s\n", buffer2) == 1)
		{
			if (strcmp(buffer2, "medium") == 0)
			{
				config->vbenchmark = &vbenchmark_medium;
				config->benchmark_name = "medium";
			}
			else if (strcmp(buffer2, "huge") == 0)
			{
				config->vbenchmark = &vbenchmark_huge;
				config->benchmark_name = "huge";
			}
			else if (strcmp(buffer2, "big") == 0)
			{
				config->vbenchmark = &vbenchmark_big;
				config->benchmark_name = "big";
			}
			else
			{
				config->vbenchmark = &vbenchmark1;
				config->benchmark_name = "mickey";
			}
		}
		else if (sscanf(buffer, "implementation = %s\n", buffer2) == 1)
		{
			if (strcmp(buffer2, "naive") == 0)
			{
				config->implem = naive;
				config->implem_name = "naive";
			}
			else if (strcmp(buffer2, "tree") == 0)
			{
				config->implem = trees;
				config->implem_name = "tree";
			}
			else if (strcmp(buffer2, "cluster") == 0)
			{
				config->implem = cluster;
				config->implem_name = "cluster";
			}
			else if (strcmp(buffer2, "SIMD") == 0)
			{
				config->implem = SIMD;
				config->implem_name = "SIMD";
			}
			else if (strcmp(buffer2, "BDPT") == 0)
			{
				config->implem = bdpt;
				config->implem_name = "BDPT";
			}
			else
			{
				config->benchmark = &benchmark1;
				config->benchmark_name = "mickey";
			}
		}
		else
		{
			fprintf(stderr, "Invalid config option line %d: %s\n", line, buffer);
			abort();
		}
	}

	// Check error
	if (!feof(fp))
	{
		perror(filename);
		abort();
	}
}

void print_config(pt_config_t *config)
{
	printf(
		""
		"┌──────────────────────────────┐\n"
		"│ PATH TRACER WITH MONTE CARLO │\n"
		"└──────────────────────────────┘\n"
		" IMAGE PARAMETERS\n"
		" ┬──────────────────────────────\n"
		" ├%-20s %d\n"
		" ├%-20s %d\n"
		" ├%-20s %zu\n"
		" └%-20s %d\n"
		" -──────────────────────────────\n"
		" MEASURE PARAMETERS\n"
		" ┬──────────────────────────────\n"
		" ├%-20s %zd\n"
		" ├%-20s %d\n"
		" └%-20s %s\n"
		" -──────────────────────────────\n"
		" SIMULATION PARAMETERS\n"
		" ┬──────────────────────────────\n"
		" ├%-20s %s\n"
		" └%-20s %s\n",
		"WIDTH", config->width,
		"HEIGHT", config->height,
		"SAMPLES", config->samples,
		"BOUNCES", config->bounces,
		"PRINT RATE", config->print_rate,
		"MEASURES", config->n_measures,
		"OUTPUT FILE", config->output_filename,
		"BENCHMARK", config->benchmark_name,
		"IMPLEMENTATION", config->implem_name);
}
