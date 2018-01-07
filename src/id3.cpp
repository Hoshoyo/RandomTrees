#include "parser.h"
#include "util.h"
#include <dynamic_array.h>

#define internal static

struct Data_Values {
	Attribute* max_attribs;		// normal calloc -> free()				array count must be num_attribs
	Attribute* min_attribs;		// normal calloc -> free()				array count must be num_attribs

	// Number of Attribute value types in this dataset
	// array length must be the same as the number of attributes
	// use free to release the memory
	u32* attribs_value_type_count;

	// Types of attributes in array [attribute index][attribute value type]
	// use free to release attribute_types and iterate using array_release
	// on the attribute value types of each attribute
	Attribute** attribute_types;

	// Count of attributes for each type value in the array [attribute_index][attribute value type]
	// use free to release attribute_types_data_count and iterate using array_release
	// on the attribyte value types of each attribute
	u32** attribute_types_data_count;

	// The number of different classes on the data set
	u32 num_classes;

	// The index of the attribute which has the biggest gain of all
	// used to create the root of the decision tree
	u32 biggest_gain_index;
};

File_Data* data_divide_on_attribute(Data_Values* data_values, File_Data* in_data);

/*
	This function returns true if entropy is different than 0 and false otherwise
*/
bool calculate_data_gains(File_Data* file_data, Data_Values* data_values);

Data_Values extract_data_from_filedata(File_Data* file_data) {
	u32 num_attribs = file_data->num_attribs;
	u32 num_entries = file_data->num_entries;

	Attribute* max_attribs = (Attribute*)calloc(num_attribs, sizeof(Attribute));
	Attribute* min_attribs = (Attribute*)calloc(num_attribs, sizeof(Attribute));
	u32* attribs_value_type_count = (u32*)calloc(num_attribs, sizeof(u32));

	// fill min and max arrays with initial values
	for (s32 i = 0; i < num_attribs; ++i) {
		switch (file_data->value_types[i]) {
		case VALUE_TYPE_INT: {
			max_attribs[i].value_int = INT_MIN;
			min_attribs[i].value_int = INT_MAX;
			max_attribs[i].type = VALUE_TYPE_INT;
			min_attribs[i].type = VALUE_TYPE_INT;
		}break;
		case VALUE_TYPE_FLOAT: {
			max_attribs[i].value_float = -(FLT_MAX);
			min_attribs[i].value_float = FLT_MAX;
			max_attribs[i].type = VALUE_TYPE_FLOAT;
			min_attribs[i].type = VALUE_TYPE_FLOAT;
		}break;
		case VALUE_TYPE_CHAR: {
			max_attribs[i].value_char = 'A';
			min_attribs[i].value_char = 'Z';
			max_attribs[i].type = VALUE_TYPE_CHAR;
			min_attribs[i].type = VALUE_TYPE_CHAR;
		}break;
		}
	}

	// Make an array in order to count how many different attribute values are for each attribute
	Attribute** attribute_aux_counter = (Attribute**)calloc(num_attribs, sizeof(Attribute*));
	u32** attribute_types_data_count = (u32**)calloc(num_attribs, sizeof(u32*));
	for (u32 i = 0; i < num_attribs; ++i) {
		attribute_aux_counter[i] = array_create(Attribute, 8);	// heuristic, expecting at most 8 different attribute values
		attribute_types_data_count[i] = array_create(u32, 8);	// heuristic, expecting at most 8 different attribute values
	}


	for (s32 n = 0; n < num_entries; ++n) {
		for (s32 i = 0; i < num_attribs; ++i) {
			size_t attrib_type_count = array_get_length(attribute_aux_counter[i]);
			bool new_attrib = true;
			if (attrib_type_count > 0) {
				for (u32 k = 0; k < attrib_type_count; ++k) {
					if (attribute_equal(&file_data->attribs[num_attribs * n + i], &attribute_aux_counter[i][k])) {
						attribute_types_data_count[i][k] += 1;
						new_attrib = false;
						break;
					}
				}
			}
			if (new_attrib) {
				array_push(attribute_aux_counter[i], &file_data->attribs[num_attribs * n + i]);

				u32 v = 1;	// start with 1
				array_push(attribute_types_data_count[i], &v);
			}

			switch (file_data->attribs[num_attribs * n + i].type) {
			case VALUE_TYPE_INT: {
				if (file_data->attribs[num_attribs * n + i].value_int > max_attribs[i].value_int)
					max_attribs[i].value_int = file_data->attribs[num_attribs * n + i].value_int;
				if (file_data->attribs[num_attribs * n + i].value_int < min_attribs[i].value_int)
					min_attribs[i].value_int = file_data->attribs[num_attribs * n + i].value_int;
			}break;
			case VALUE_TYPE_FLOAT: {
				if (file_data->attribs[num_attribs * n + i].value_float > max_attribs[i].value_float)
					max_attribs[i].value_float = file_data->attribs[num_attribs * n + i].value_float;
				if (file_data->attribs[num_attribs * n + i].value_float < min_attribs[i].value_float)
					min_attribs[i].value_float = file_data->attribs[num_attribs * n + i].value_float;
			}break;
			case VALUE_TYPE_CHAR: {
				if (file_data->attribs[num_attribs * n + i].value_char > max_attribs[i].value_char)
					max_attribs[i].value_char = file_data->attribs[num_attribs * n + i].value_char;
				if (file_data->attribs[num_attribs * n + i].value_char < min_attribs[i].value_char)
					min_attribs[i].value_char = file_data->attribs[num_attribs * n + i].value_char;
			}break;
			}
		}
	}
	u32 num_attrib_type_values = 0;
	for (u32 i = 0; i < num_attribs; ++i) {
		size_t count = array_get_length(attribute_aux_counter[i]);
		attribs_value_type_count[i] = count;
		if (i != file_data->class_index)
			num_attrib_type_values += count;
	}
	u32 num_classes = attribs_value_type_count[file_data->class_index];

	Data_Values result;
	result.max_attribs = max_attribs;
	result.min_attribs = min_attribs;
	result.attribute_types = attribute_aux_counter;
	result.attribs_value_type_count = attribs_value_type_count;
	result.num_classes = num_classes;
	result.attribute_types_data_count = attribute_types_data_count;
	//calculate_data_gains(file_data, &result);

	return result;
}

internal s32 get_value_index(Attribute* attrib, u32 attrib_index, Attribute** attribute_types){
	s32 length = array_get_length(attribute_types[attrib_index]);
	assert(length > 0);
	for(s32 i = 0; i < length; ++i){
		if(attribute_equal(attrib, &attribute_types[attrib_index][i])){
			return i;
		}
	}
	return -1;
}

internal u32 get_index(u32 x, u32 y, u32 z, u32 y_length, u32 z_length) {
	return (x * y_length * z_length) + y * z_length + z;
}

bool calculate_data_gains(File_Data* file_data, Data_Values* data_values) {
	u32 class_number = data_values->num_classes;
	u32 data_length = file_data->num_entries;

	// Calculate counts for the data set
	u32* class_count = (u32*)calloc(class_number, sizeof(u32));
	
	{
		for (u32 i = 0; i < data_length; ++i) {
			for (u32 c = 0; c < class_number; ++c) {
				if (attribute_equal(&file_data->attribs[i * file_data->num_attribs + file_data->class_index], &data_values->attribute_types[file_data->class_index][c])) {
					class_count[c] += 1;
				}
			}
		}
	}

	// Calculate the Info(D) of the whole thing
	r32 info_d = 0.0f;
	for (u32 i = 0; i < class_number; ++i) {
		r32 p_i = (r32)class_count[i] / (r32)data_length;
		info_d += -(p_i * math_log(p_i) / math_log(2.0f));
	}
	
	//u32 attrb_tempo_count_per_class[file_data->num_attribs][TEMPO_COUNT][class_number] = {};
	u32 max_attrib_value_count = 0;
	for (u32 i = 0; i < file_data->num_attribs; ++i) {
		if (data_values->attribs_value_type_count[i] > max_attrib_value_count)
			max_attrib_value_count = data_values->attribs_value_type_count[i];
	}

	u32* attrb_count_per_class = (u32*)calloc(1, file_data->num_attribs * max_attrib_value_count * class_number * sizeof(u32));
	for(u32 i = 0; i < data_length; ++i){
		for(u32 a = 0; a <  file_data->num_attribs; ++a){
			if(a == file_data->class_index)
				continue;
			s32 value_index = get_value_index(&file_data->attribs[i * file_data->num_attribs + a], a, data_values->attribute_types);
			s32 class_index = get_value_index(&file_data->attribs[i * file_data->num_attribs + file_data->class_index], file_data->class_index, data_values->attribute_types);
			u32 in = get_index(a, value_index, class_index, max_attrib_value_count, class_number);
			attrb_count_per_class[in] += 1;
			assert(value_index != -1);	
		}
	}

	bool result = false;

	r32 max_gain = FLT_MIN;
	for (u32 a = 0; a < file_data->num_attribs; ++a) {
		if(a == file_data->class_index)
			continue;
		r32 info_d_subattrib = 0.0f;
		u32 num_value_types = array_get_length(data_values->attribute_types[a]);
		for (u32 i = 0; i < num_value_types; ++i) {
			r32 t = 0.0f;
			for (u32 j = 0; j < class_number; ++j) {
				u32 in = get_index(a, i, j, max_attrib_value_count, class_number);
				r32 p_i = (r32)attrb_count_per_class[in] / (r32)data_values->attribute_types_data_count[a][i];
				if (p_i == 0) continue;
				t += -(p_i * math_log(p_i) / math_log(2.0f));
			}
			info_d_subattrib += ((r32)data_values->attribute_types_data_count[a][i] / (r32)data_length) * t;
		}
		if(info_d - info_d_subattrib > max_gain){
			max_gain = info_d - info_d_subattrib;
			data_values->biggest_gain_index = a;
			result = true;
		}
		printf("infod info_d_subattrib %d: %f\n", a, info_d - info_d_subattrib);
	}
	if(result)
		printf("Biggest gain index = %d\n", data_values->biggest_gain_index);
	return result;
}

File_Data* data_divide_on_attribute(Data_Values* data_values, File_Data* in_data) {
	File_Data* result = 0;

	u32 num_entries = in_data->num_entries;
	u32 num_attribs = in_data->num_attribs;
	u32 biggest_gain_index = data_values->biggest_gain_index;
	u32 num_value_types_of_biggest_gain = data_values->attribs_value_type_count[biggest_gain_index];
	// divide root being biggest_gain_index and num_value_types_of_biggest_gain branches
	
	result = array_create(File_Data, num_value_types_of_biggest_gain);
	array_allocate(result, num_value_types_of_biggest_gain);

	for (u32 i = 0; i < num_value_types_of_biggest_gain; ++i) {
		result[i].attribs = array_create(Attribute, 16);
		result[i].num_attribs = num_attribs - 1;
		result[i].num_entries = 0;
		// The class index is one less if it was originally after the biggest_gain_index, otherwise stays the same
		result[i].class_index = (in_data->class_index <= biggest_gain_index) ? in_data->class_index : in_data->class_index - 1;
		result[i].integrity = true;
		result[i].value_types = (u32*)calloc(num_attribs - 1, sizeof(u32));
		for (u32 j = 0, k = 0; j < num_attribs; ++k) {
			if (k != biggest_gain_index) {
				result[i].value_types[j] = in_data->value_types[k];
				++j;
			}
		}
	}

	for (u32 i = 0; i < num_entries; ++i) {
		for (u32 n = 0; n < num_attribs; ++n) {
			if (n == biggest_gain_index) {
				s32 index = in_data->attribs[i * num_attribs + n].value_int;
				assert(index <= num_value_types_of_biggest_gain);
				result[index].num_entries += 1;
				for (u32 sub = 0; sub < num_attribs; ++sub) {
					if (sub != biggest_gain_index) {
						array_push(result[index].attribs, &in_data->attribs[i * num_attribs + sub]);
					}
				}
			}
		}
	}

	return result;
}

#undef internal
