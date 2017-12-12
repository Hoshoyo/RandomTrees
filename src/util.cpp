#include "util.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

extern void print(const s8* format, ...)
{
	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

extern void* memory_alloc(s32 size)
{
	return malloc(size);
}

extern void memory_free(void* block)
{
	free(block);
}

extern s32 random_number()
{
	return rand();
}

extern void flush_stream()
{
	fflush(stdout);
}

bool c_str_equal(const char* str1, const char* str2)
{
	const char* ptr = str1;
	const char* ptr2 = str2;
	while (*ptr) {
		if (*ptr++ != *ptr2++)
			return false;
	}

	if (*ptr2 == 0)
		return true;
	else
		return false;
}

s32 str_to_s32(char* text, int length)
{
	s32 result = 0;
	s32 tenths = 1;
	for (int i = length - 1; i >= 0; --i, tenths *= 10)
		result += (text[i] - 0x30) * tenths;
	return result;
}

s32 str_length(char* text) {
	s32 length = 0;
	while (*text++) ++length;
	return length;
}


#if defined(_WIN64) || defined(_WIN32)

extern void* file_read_entire(FILE_HANDLE file, size_t file_size, void* mem)
{
	u64 maxs32 = 0x7fffffff;
	DWORD bytes_read = 0;
	void* memory = mem;
	if (file_size == 0)
		return 0;
	if (!mem)
		memory = memory_alloc(file_size + 1);
	if (INVALID_HANDLE_VALUE == file)
		return 0;

	((char*)memory)[file_size] = 0;

	if (file_size > maxs32)
	{
		void* mem_aux = memory;
		s64 num_reads = file_size / maxs32;
		s64 rest = file_size % maxs32;
		DWORD bytes_read;
		for (s64 i = 0; i < num_reads; ++i)
		{
			ReadFile(file, mem_aux, (DWORD)maxs32, &bytes_read, 0);
			mem_aux = (char*)mem_aux + maxs32;
		}
		ReadFile(file, mem_aux, (DWORD)rest, &bytes_read, 0);
	}
	else
	{
		ReadFile(file, memory, (DWORD)file_size, &bytes_read, 0);
	}

	return memory;
}

extern u64 file_get_size(const char* filename)
{
	WIN32_FIND_DATA info;
	HANDLE search_handle = FindFirstFileEx(filename, FindExInfoStandard, &info, FindExSearchNameMatch, NULL, FIND_FIRST_EX_LARGE_FETCH);
	if (search_handle == INVALID_HANDLE_VALUE)
		return -1;
	FindClose(search_handle);
	u64 size = 0;
	size = (u64)info.nFileSizeLow | ((u64)info.nFileSizeHigh << 32);
	return size;
}

extern FILE_HANDLE file_open(const char* filename, u32 access_flags)
{
	HANDLE handle = CreateFile(filename, access_flags, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	return handle;
}

extern void file_close(FILE_HANDLE file) {
	CloseHandle(file);
}

#elif defined(__linux__)
extern void* file_read_entire(FILE_HANDLE file, size_t file_size, void* mem)
{
	void* memory = mem;
	if(file_size == 0) return 0;
	if(mem == 0){
		memory = memory_alloc(file_size);
	}
	int error = read(file, memory, file_size);
	if(error == -1){
		return 0;
	}
	return memory;
}

extern u64 file_get_size(const char* filename)
{
	int filedescriptor = open(filename, O_RDONLY);
	struct stat buf;
	fstat(filedescriptor, &buf);
	size_t size = buf.st_size;
	close(filedescriptor);
	return size;
}

extern FILE_HANDLE file_open(const char* filename, u32 access_flags)
{	
	return open(filename, access_flags);
}

extern void file_close(FILE_HANDLE file)
{
	close(file);
}
#endif
