#pragma once
#include <stdlib.h>
#include <string.h>

#if defined(_WIN64) || defined(_WIN32)
#define HO_API __stdcall
#else
#define HO_API
#endif

#ifdef __cplusplus
#define HO_EXTERN extern "C"
#else
#define HO_EXTERN extern
#endif

#define array_create(type, num) (type*)_array_create(num, sizeof(type))
#define array_push(arr, data) _array_push((void**)&arr, data)
#define array_emplace(arr) _array_emplace((void**)&arr)
#define array_allocate(arr, num) _array_allocate((void**)&arr, num)

HO_EXTERN void HO_API _array_allocate(void** array_, size_t num_elements);
HO_EXTERN void* HO_API _array_create(int num_elements, size_t size);
HO_EXTERN void HO_API _array_push(void** array_, void* data);
HO_EXTERN size_t HO_API _array_emplace(void** array_);

HO_EXTERN size_t HO_API array_get_capacity(void* array_);
HO_EXTERN size_t HO_API array_get_length(void* array_);
HO_EXTERN size_t HO_API array_get_element_size(void* array_);
HO_EXTERN   void HO_API array_pop(void* array);
HO_EXTERN   void HO_API array_release(void* array);
HO_EXTERN   void HO_API array_remove(void* array_, size_t index);
HO_EXTERN   void HO_API array_clear(void* array_);
HO_EXTERN   void HO_API array_scramble(void* array_);

#if defined(DYNAMIC_ARRAY_IMPLEMENT)

typedef struct array_t {
	size_t capacity;
	size_t length;
	size_t size_element;
} array;

HO_EXTERN void HO_API array_pop(void* array_)
{
	array* base = (array*)((char*)array_ - sizeof(array));
	if (base->length > 0)
		base->length--;
}

HO_EXTERN void HO_API array_release(void* array_)
{
	array* base = (array*)((char*)array_ - sizeof(array));
	free(base);
}

HO_EXTERN void HO_API array_remove(void* array_, size_t index)
{
	array* base = (array*)((char*)array_ - sizeof(array));
	size_t capacity = base->capacity;
	size_t length = base->length;
	size_t size_element = base->size_element;

	void* dst = (char*)array_ + index * size_element;
	void* from = (char*)array_ + ((length - 1) * size_element);
	memcpy(dst, from, size_element);
	base->length--;
}

HO_EXTERN void* HO_API _array_create(int num_elements, size_t size)
{
	void* arr = malloc(sizeof(array) + num_elements * size);
	array* aux = (array*)arr;
	aux->capacity = num_elements;
	aux->length = 0;
	aux->size_element = size;
	return (char*)arr + sizeof(array);
}

HO_EXTERN void HO_API _array_push(void** array_, void* data)
{
	array* base = (array*)((char*)*array_ - sizeof(array));
	size_t capacity = base->capacity;
	size_t length = base->length;
	size_t size_element = base->size_element;

	if (length == capacity)
	{
		capacity *= 2;
		base->capacity = capacity;

		void* new_mem = realloc(base, capacity * size_element + sizeof(array));
		base = (array*)new_mem;
		*array_ = (char*)new_mem + sizeof(array);
	}

	void* dst = (char*)*array_ + length * size_element;
	base->length++;
	memcpy(dst, data, size_element);
}

HO_EXTERN size_t HO_API _array_emplace(void** array_)
{
	array* base = (array*)((char*)*array_ - sizeof(array));
	size_t capacity = base->capacity;
	size_t length = base->length;
	size_t size_element = base->size_element;

	if (length == capacity)
	{
		capacity *= 2;
		base->capacity = capacity;
		void* new_mem = realloc(base, capacity * size_element + sizeof(array));
		base = (array*)new_mem;
		*array_ = (char*)new_mem + sizeof(array);
	}

	void* dst = (char*)*array_ + length * size_element;
	base->length++;

	return length;
}

HO_EXTERN void HO_API _array_allocate(void** array_, size_t num_elements) 
{
	array* base = (array*)((char*)*array_ - sizeof(array));
	size_t capacity = base->capacity;
	size_t length = base->length;
	size_t size_element = base->size_element;

	if (length == capacity || length + num_elements > capacity)
	{
		capacity += num_elements;
		base->capacity = capacity;

		void* new_mem = realloc(base, capacity * size_element + sizeof(array));
		base = (array*)new_mem;
		*array_ = (char*)new_mem + sizeof(array);
	}
	base->length += num_elements;
}

HO_EXTERN void HO_API array_scramble(void* array_) 
{
	size_t size_element = ((array*)((char*)array_ - sizeof(array)))->size_element;
	size_t length = ((array*)((char*)array_ - sizeof(array)))->length;
	void* mem_aux = malloc(size_element);

	for (int i = 0; i < length; ++i) {
		size_t r = random_number() % length;
		memcpy(mem_aux, ((char*)array_ + i * size_element), size_element);
		memcpy((char*)array_ + i * size_element, (char*)array_ + r * size_element, size_element);
		memcpy((char*)array_ + r * size_element, mem_aux, size_element);
	}
	free(mem_aux);
}

HO_EXTERN size_t HO_API array_get_capacity(void* array_)
{
	return ((array*)((char*)array_ - sizeof(array)))->capacity;
}

HO_EXTERN size_t HO_API array_get_length(void* array_)
{
	return ((array*)((char*)array_ - sizeof(array)))->length;
}

HO_EXTERN size_t HO_API array_get_element_size(void* array_)
{
	return ((array*)((char*)array_ - sizeof(array)))->size_element;
}

HO_EXTERN void HO_API array_clear(void* array_)
{
	((array*)((char*)array_ - sizeof(array)))->length = 0;
}
#endif	// DYNAMIC_ARRAY_IMPLEMENT

#undef HO_API
