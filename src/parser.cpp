#include "parser.h"
#include "util.h"
#define DYNAMIC_ARRAY_IMPLEMENT
#include <dynamic_array.h>

#define internal static

internal s64 str_to_s64(char* text, int length)
{
	s64 result = 0;
	s64 tenths = 1;
	for (int i = length - 1; i >= 0; --i, tenths *= 10)
		result += (text[i] - 0x30) * tenths;
	return result;
}

internal r64 str_to_r64(char* text, int length)
{
	r64 result = 0.0;
	r64 tenths = 1.0;
	r64 frac_tenths = 0.1;
	int point_index = 0;

	while (text[point_index] != '.' && point_index < length)
		++point_index;

	for (int i = point_index - 1; i >= 0; --i, tenths *= 10.0)
		result += (text[i] - 0x30) * tenths;
	for (int i = point_index + 1; i < length; ++i, frac_tenths *= 0.1)
		result += (text[i] - 0x30) * frac_tenths;
	return result;
}

internal r32 str_to_r32(char* text, int length)
{
	r32 result = 0.0;
	r32 tenths = 1.0;
	r32 frac_tenths = 0.1;
	int point_index = 0;

	while (text[point_index] != '.' && point_index < length)
		++point_index;

	for (int i = point_index - 1; i >= 0; --i, tenths *= 10.0)
		result += (text[i] - 0x30) * tenths;
	for (int i = point_index + 1; i < length; ++i, frac_tenths *= 0.1)
		result += (text[i] - 0x30) * frac_tenths;
	return result;
}

internal bool is_number(u8 c) {
	return (c >= '0' && c <= '9');
}

internal bool is_letter(u8 c) {
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

internal bool is_white_space(char str)
{
	if (str == ' ' ||
		str == '\t' ||
		str == '\n' ||
		str == '\v' ||
		str == '\f' ||
		str == '\r')
	{
		return true;
	}
	return false;
}

internal u8 uppercase(u8 c) {
	if (c >= 'a' && c <= 'z') {
		return c - 0x20;
	}
	return c;
}

const u32 TOKEN_END_OF_STREAM = 0;
const u32 TOKEN_NUMBER_FLOAT = 1;
const u32 TOKEN_NUMBER_INT = 2;
const u32 TOKEN_COMMA = 3;
const u32 TOKEN_CHARACTER = 4;

internal Token* tokenize(char* filename) 
{
	FILE_HANDLE handle = file_open(filename, READ_ONLY);
	if (handle == INVALID_FILE) {
		print("Could not open file %s\n", filename);
		assert(0);
	}
	size_t file_size = file_get_size(filename);
	u8* data = (u8*)file_read_entire(handle, file_size, 0);
	file_close(handle);

	Token* tokens = array_create(Token, file_size * sizeof(Token));
	u32 token_count = 0;

	// Lexer
	u8* start = data;
	u8* at = data;

	while (1) {
		Token* t = &tokens[token_count];
		t->data = (s8*)at;

		u32 length = 1;
		u8 c = *at;

		if (c == 0) {
			t->type = TOKEN_END_OF_STREAM;
			break;
		} else if (is_white_space(c)) {

			at += 1;
			continue;
		} else if (is_number(c)) {
			bool floating_point = false;
			int i = 0;
			length = 0;
			do {
				++i;
				length++;
				if (at[i] == '.' && !floating_point) {
					floating_point = true;
					++i;
					length++;
				}
			} while (at[i] && (is_number(at[i])));
			t->type = (floating_point) ? TOKEN_NUMBER_FLOAT : TOKEN_NUMBER_INT;
		} else if (c == '.') {
			int i = 0;
			length = 0;
			do {
				++i;
				length++;
			} while (at[i] && (is_number(at[i])));
			t->type = TOKEN_NUMBER_FLOAT;
		} else if (c == ',') {
			t->type = TOKEN_COMMA;
		} else if (is_letter(c)) {
			t->type = TOKEN_CHARACTER;
		}

		t->length = length;
		at += length;
		token_count += 1;
	}
	return tokens;
}

internal Token* peek_token(Token* tokens, s32 count) {
	return &tokens[count];
}

#define EAT_COMMA if (t->type != TOKEN_COMMA) {	\
print("Error parsing line %d expected comma but got %.*s\n", line, t->length, t->data);	\
assert(0); break;	\
}	\
t = &tokens[++token_count]

internal s32 attribute_struct_size_get(Token* tokens, s32 attribs_num) {
	s32 size = 0;
	s32 count = 0;
	for (s32 i = 0; i < attribs_num;) {
		bool end = false;

		switch (peek_token(tokens, count)->type) {
			case TOKEN_END_OF_STREAM: end = true; break;
			case TOKEN_NUMBER_INT: {
				size += sizeof(int);
				++i;
			}break;
			case TOKEN_NUMBER_FLOAT: {
				size += sizeof(float);
				++i;
			}break;
			case TOKEN_COMMA: break;
			case TOKEN_CHARACTER: {
				size += sizeof(char);
				++i;
			}break;
		}
		count += 1;
		if (end) break;
	}
	return size;
}

// this function updates the VALUE_TYPE of a specified attribute, only INTEGERS are upgraded to FLOATS
internal void update_value_types(u32* value_types, s32 attrib_index, u32 value) {
	if (value_types[attrib_index] == VALUE_TYPE_INT && value == VALUE_TYPE_FLOAT) {
		value_types[attrib_index] = value;
	} else if (value_types[attrib_index] == VALUE_TYPE_FLOAT && value == VALUE_TYPE_INT) {
		return;
	} else {
		value_types[attrib_index] = value;
	}

}

// this function return true if an Attribute is equal to another
// and false otherwise
bool attribute_equal(Attribute* a1, Attribute* a2) {
	switch (a1->type) {
	case VALUE_TYPE_INT:
		return (a1->value_int == a2->value_int);
	case VALUE_TYPE_FLOAT:
		return (a1->value_float == a2->value_float);
	case VALUE_TYPE_CHAR:
		return (a1->value_char == a2->value_char);
	default:
		return false;
	}
}

/*
		This function upgrades integer values to float values if one of the fields
	 in the data file suggests that field is a real number instead of an integer
	 it also calculates the max and min attributes for later normalization

	 line_attribs		: (in) the array of parsed attributes
	 value_types		: (in) the array that contains the enum Value_Type of an attribute
	 num_attribs		: (in) number of attributes of an instance including the class
	 max_attribs		: (in/out) the array to be filled with the maximum value of each attribute
	 min_attribs		: (in/out) the array to be filled with the minimum value of each attribute
	 attribs_count		: (in/out) the array to be filled with the number of different values for each attribute
*/

internal void convert_attribute_types(Attribute* line_attribs, u32* value_types, s32 num_attribs) {
	//
	// @TODO discretize real values
	//
	size_t num_entries = array_get_length(line_attribs);
	for (s32 n = 0; n < num_entries; ++n) {
		for (s32 i = 0; i < num_attribs; ++i) {
			if (line_attribs[num_attribs * n + i].type == VALUE_TYPE_INT && value_types[i] == VALUE_TYPE_FLOAT) {
				s32 v = line_attribs[num_attribs * n + i].value_int;
				line_attribs[num_attribs * n + i].value_float = (r32)v;
				line_attribs[num_attribs * n + i].type = VALUE_TYPE_FLOAT;
			}
		}
	}
}

/*
		The parsed Attributes of File_Data are divided in arrays of attribs_num fields representing 
	the class and the attributes. Those arrays are created for each entry in the data contiguously.

	Example for 4 fields and 3 entries (Attribute struct has 8 bytes):

	0x00 Attribute 1 { VALUE_TYPE_INT 10, VALUE_TYPE_INT 20, VALUE_TYPE_INT 30, INT 40 }
	0x20 Attribute 2 { VALUE_TYPE_INT 50, VALUE_TYPE_INT 60, VALUE_TYPE_INT 70, INT 80 }
	0x40 Attribute 3 { VALUE_TYPE_INT 90, VALUE_TYPE_INT 91, VALUE_TYPE_INT 92, INT 93 }

	filename		: the path of the file to be parsed
	attribs_num		: the number of attributes that the file contains per entry, including the class
	class_index		: the index of the class attribute in the file

	Return value:
		A filled File_Data structure containing max and min values, all the data parsed into Attributes and
	line information aswell as the number of attributes including the class.
	the field integrity is false if an error occurred while parsing the file and true otherwise

	Remarks:
		- The fields max_attribs and min_attribs are allocated using calloc and must be freed after use with free
		- The field attribs is allocated using array_create and must be freed after use with array_release
	
*/
static s32 get_integer_attribute_indexed_value(s32 attrib, s32** integer_index_values_array, s32 length, s32 index, File_Data* fdata) {
	if (integer_index_values_array[index] == 0)
		integer_index_values_array[index] = array_create(s32, 8);
	size_t size = array_get_length(integer_index_values_array[index]);

	if (size > 0) {
		for (size_t i = 0; i < size; ++i) {
			if (integer_index_values_array[index][i] == attrib)
				return i;
		}
		fdata->count_value_types_each_attribute[index] += 1;
		array_push(integer_index_values_array[index], &attrib);
		return (s32)size;
	} else {
		fdata->count_value_types_each_attribute[index] += 1;
		array_push(integer_index_values_array[index], &attrib);
		return 0;
	}
	return -1;
}

extern File_Data parse_file(s8* filename, s32 attribs_num, s32 class_index) {
	File_Data result = {};
	Token* tokens = tokenize(filename);
	s32 token_count = 0;
	s32 lines = 0;

	Attribute* line_attribs = (Attribute*)_array_create(16, sizeof(Attribute) * attribs_num);

	result.count_value_types_each_attribute = (u32*)calloc(attribs_num, sizeof(u32*));

	u32* value_types = (u32*)calloc(attribs_num, sizeof(u32));

	result.integer_index_value = (s32**)calloc(attribs_num, sizeof(s32**));

	Token* t = &tokens[0];
	bool end = false;
	for (s32 line = 0; !end; ++line) {
		if (t->type == TOKEN_END_OF_STREAM) break;
		size_t index = array_emplace(line_attribs) * attribs_num;
		for (s32 i = 0; i < attribs_num; ++i) {
			switch (t->type) {
			case TOKEN_END_OF_STREAM: {
				print("Error parsing attribute %d, unexpected end of stream in line %d\n", attribs_num + 1, line);
				assert(0);
				return {};
			}break;
			case TOKEN_NUMBER_INT: {
				s32 value = str_to_s32(t->data, t->length);
				line_attribs[index + i].value_int = get_integer_attribute_indexed_value(value, result.integer_index_value, attribs_num, i, &result);
				//line_attribs[index + i].value_int = value;
				line_attribs[index + i].type = VALUE_TYPE_INT;
				update_value_types(value_types, i, VALUE_TYPE_INT);
			}break;
			case TOKEN_NUMBER_FLOAT: {
				r32 value = str_to_r32(t->data, t->length);
				line_attribs[index + i].value_float = value;
				line_attribs[index + i].type = VALUE_TYPE_FLOAT;
				update_value_types(value_types, i, VALUE_TYPE_FLOAT);
			}break;
			case TOKEN_CHARACTER: {
				s8 value = uppercase(t->data[0]);
				line_attribs[index + i].value_char = value;
				line_attribs[index + i].type = VALUE_TYPE_CHAR;
				update_value_types(value_types, i, VALUE_TYPE_CHAR);
			}break;
			default: {
				print("Error parsing attribute %d in line %d expected int but got %.*s\n", attribs_num + 1, line, t->length, t->data);
				assert(0);
				return {};
			}break;
			}
			

			t = &tokens[++token_count];

			if (i + 1 < attribs_num || peek_token(tokens, token_count)->type == TOKEN_COMMA) {
				EAT_COMMA;
			}
		}
		lines += 1;
	}
	//for (s32 i = 0; i < attribs_num; ++i) {
	//	if (integer_index_value[i] != 0)
	//		array_release(integer_index_value[i]);
	//}
	//free(integer_index_value);

	convert_attribute_types(line_attribs, value_types, attribs_num);

	result.types_indexes_remove_history = array_create(s32, attribs_num);
	for (s32 i = 0; i < attribs_num; ++i) {
		s32 minus_one = -1;
		array_push(result.types_indexes_remove_history, &minus_one);
	}

	array_release(tokens);
	result.num_max_attributes = attribs_num;
	result.tree_split_original_index = -1;
	result.attribs = line_attribs;
	result.num_attribs = attribs_num;
	result.num_entries = lines;
	result.integrity = true;
	result.class_index = class_index;
	result.value_types = value_types;
	return result;
}

#undef internal
