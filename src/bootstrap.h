#pragma once
#include "common.h"
#include "parser.h"

typedef struct
{
	File_Data training_set;
	File_Data test_set;
} Bootstrap;

extern Bootstrap* bootstrap(File_Data original_file_data, s32 num_bootstraps);
extern void print_bootstraps(Bootstrap* bootstraps);