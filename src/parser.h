#pragma once
#include "common.h"

#define INT_MIN (-INT_MAX - 1)
#define INT_MAX 2147483647
#define FLT_MAX          3.402823466e+38F        // max value
#define FLT_MIN          1.175494351e-38F        // min normalized positive value

typedef struct Token_T{
	char* data;
	u32   length;
	u32   type;
} Token;

enum Value_Type {
	VALUE_TYPE_NONE = -1,
	VALUE_TYPE_INT,
	VALUE_TYPE_FLOAT,
	VALUE_TYPE_CHAR,
	VALUE_TYPE_BOOL,
};

struct Attribute {
	Value_Type type;
	union {
		r32 value_float;
		s32 value_int;
		bool value_bool;
		s8 value_char;
	};
};

struct File_Data {
	// Exemplo arquivo 4 atributos e 10 entries
	// Pega 10 random com reposicao e monta um bootstrap
	// Monta uma quantidade X de bootstraps cada um vai virar
	// Uma árvore de decisão. Ou seja, florestas aleatórias
	// [A A A A   B B B B    C C C C ...]
	// [A A A A   A A A A    C C C C ...]  [B B B B]
	Attribute* attribs;			// dynamic array -> array_release()		array count must be num_attribs * num_entries

	s32 num_attribs;
	s32 class_index;
	s32 num_entries;
	u32* value_types;			// normal calloc -> free()				array count must be num_attribs
	u32* count_value_types_each_attribute;

	u32 num_max_attributes;
	s32* types_indexes_remove_history;
	s32 tree_split_original_index;

	bool integrity;
};

extern File_Data parse_file(s8* filename, s32 attribs_num, s32 class_index);
extern bool attribute_equal(Attribute* a1, Attribute* a2);