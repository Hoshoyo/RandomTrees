#pragma once
#include "common.h"

typedef struct Token_T{
	char* data;
	u32   length;
	u32   type;
} Token;

enum Value_Type {
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
	Attribute* max_attribs;		// normal calloc -> free()
	Attribute* min_attribs;		// normal calloc -> free()
	Attribute* attribs;			// dynamic array -> array_release()

	s32 num_attribs;
	s32 lines;
	bool integrity;
};

extern File_Data parse_file(s8* filename, s32 attribs_num);