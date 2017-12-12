#pragma once
#include "common.h"

#ifdef __linux__
#include <signal.h>
#define __debugbreak() raise(SIGTRAP)
#endif
#define assert(X) if(!(X)) __debugbreak()

extern void print(const s8* format, ...);
extern void* memory_alloc(s32 size);
extern s32 random_number();
extern void memory_free(void* block);
extern void flush_stream();

#if defined(_WIN64) || defined(_WIN32)
#include <Windows.h>
typedef HANDLE FILE_HANDLE;

#define INVALID_FILE INVALID_HANDLE_VALUE

#define READ_ONLY GENERIC_READ
#define WRITE_ONLY GENERIC_WRITE
#define READ_WRITE (GENERIC_READ | GENERIC_WRITE)

#elif __linux__
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#define INVALID_FILE -1
#define READ_ONLY O_RDONLY
#define WRITE_ONLY O_WRONLY
#define READ_WRITE O_RDWR

typedef int FILE_HANDLE;
#endif

extern void*       file_read_entire(FILE_HANDLE file, size_t file_size, void* mem);
extern u64         file_get_size(const char* filename);
extern FILE_HANDLE file_open(const char* filename, u32 access_flags);
extern void        file_close(FILE_HANDLE file);
extern bool c_str_equal(const char* str1, const char* str2);
extern s32 str_to_s32(char* text, int length);

s32 str_to_s32(char* text, int length);
s32 str_length(char* text);


typedef struct PARAM_CONFIG
{
    float alpha;
    float lambda;
    int qtd_camadas; // Qtd. camadas ocultas
    int qtd_neuronios;
} ParamConfig;