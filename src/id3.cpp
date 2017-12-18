#include "parser.h"
#include "util.h"
#include <dynamic_array.h>

#if 0
typedef bool Class_Type;

enum Tempo {
	TEMPO_ENSOLARADO = 0,
	TEMPO_NUBLADO,
	TEMPO_CHUVOSO,
	TEMPO_COUNT,
};

enum Temperatura {
	TEMPERATURA_QUENTE = 0,
	TEMPERATURA_AMENA,
	TEMPERATURA_FRIA,
	TEMPERATURA_COUNT,
};

enum Umidade {
	UMIDADE_ALTA = 0,
	UMIDADE_NORMAL,
	UMIDADE_COUNT,
};

enum Ventoso {
	VENTOSO_SIM = 0,
	VENTOSO_NAO,
	VENTOSO_COUNT,
};

struct Data {
	Tempo       tempo;
	Temperatura temperatura;
	Umidade     umidade;
	Ventoso     ventoso;
	bool        joga;	// classe
};

/*
Tempo			Temperatura	Umidade		Ventoso			Joga
1 Ensolarado		Quente		Alta		Falso			Nao
2 Ensolarado		Quente		Alta		Verdadeiro		Nao
3 Nublado			Quente		Alta		Falso			Sim
4 Chuvoso			Amena		Alta		Falso			Sim
5 Chuvoso			Fria		Normal		Falso			Sim
6 Chuvoso			Fria		Normal		Verdadeiro		Nao
7 Nublado			Fria		Normal		Verdadeiro		Sim
8 Ensolarado		Amena		Alta		Falso			Nao
9 Ensolarado		Fria		Normal		Falso			Sim
A Chuvoso			Amena		Normal		Falso			Sim
B Ensolarado		Amena		Normal		Verdadeiro		Sim
C Nublado			Amena		Alta		Verdadeiro		Sim
D Nublado			Quente		Normal		Falso			Sim
E Chuvoso			Amena		Alta		Verdadeiro		Nao
*/

Data test_data[] = {
	{ TEMPO_ENSOLARADO, TEMPERATURA_QUENTE, UMIDADE_ALTA,   VENTOSO_NAO, false },
	{ TEMPO_ENSOLARADO, TEMPERATURA_QUENTE, UMIDADE_ALTA,   VENTOSO_SIM, false },
	{ TEMPO_NUBLADO,    TEMPERATURA_QUENTE, UMIDADE_ALTA,   VENTOSO_NAO, true },
	{ TEMPO_CHUVOSO,    TEMPERATURA_AMENA,  UMIDADE_ALTA,   VENTOSO_NAO, true },
	{ TEMPO_CHUVOSO,    TEMPERATURA_FRIA,   UMIDADE_NORMAL, VENTOSO_NAO, true },
	{ TEMPO_CHUVOSO,    TEMPERATURA_FRIA,   UMIDADE_NORMAL, VENTOSO_SIM, false },
	{ TEMPO_NUBLADO,    TEMPERATURA_FRIA,   UMIDADE_NORMAL, VENTOSO_SIM, true },
	{ TEMPO_ENSOLARADO, TEMPERATURA_AMENA,  UMIDADE_ALTA,   VENTOSO_NAO, false },
	{ TEMPO_ENSOLARADO, TEMPERATURA_FRIA,   UMIDADE_NORMAL, VENTOSO_NAO, true },
	{ TEMPO_CHUVOSO,    TEMPERATURA_AMENA,  UMIDADE_NORMAL, VENTOSO_NAO, true },
	{ TEMPO_ENSOLARADO, TEMPERATURA_AMENA,  UMIDADE_NORMAL, VENTOSO_SIM, true },
	{ TEMPO_NUBLADO,    TEMPERATURA_AMENA,  UMIDADE_ALTA,   VENTOSO_SIM, true },
	{ TEMPO_NUBLADO,    TEMPERATURA_QUENTE, UMIDADE_NORMAL, VENTOSO_NAO, true },
	{ TEMPO_CHUVOSO,    TEMPERATURA_AMENA,  UMIDADE_ALTA,   VENTOSO_SIM, false },
};

u32 data_class_count(Data* data, u32 data_length, bool c) {
	u32 counter = 0;
	for (u32 i = 0; i < data_length; ++i) {
		if (data[i].joga == c)
			counter += 1;
	}
	return counter;
}

// return 
void test(File_Data* data) {

	const u32 data_length = sizeof(test_data) / sizeof(Data);
	const u32 class_number = 2;
	const u32 attrib_number = 4;
	Class_Type classes[class_number] = { true, false };

	// Calculate counts for the data set
	u32 class_count[class_number] = {};
	for (u32 i = 0; i < class_number; ++i)
		class_count[i] = data_class_count(test_data, data_length, classes[i]);

	u32 attrb_tempo_count[TEMPO_COUNT] = {};
	u32 attrb_temperatura_count[TEMPERATURA_COUNT] = {};
	u32 attrb_umidade_count[UMIDADE_COUNT] = {};
	u32 attrb_ventoso_count[VENTOSO_COUNT] = {};

	u32 attrb_tempo_count_per_class[TEMPO_COUNT][class_number] = {};
	u32 attrb_temperatura_count_per_class[TEMPERATURA_COUNT][class_number] = {};
	u32 attrb_umidade_count_per_class[UMIDADE_COUNT][class_number] = {};
	u32 attrb_ventoso_count_per_class[VENTOSO_COUNT][class_number] = {};

	for (u32 i = 0; i < data_length; ++i) {
		for (u32 a = 0; a < TEMPO_COUNT; ++a) {
			bool is_tempo = test_data[i].tempo == a;
			attrb_tempo_count[a] += (is_tempo) ? 1 : 0;
			attrb_tempo_count_per_class[a][is_tempo && !(test_data[i].joga)] += is_tempo;
		}
		for (u32 a = 0; a < TEMPERATURA_COUNT; ++a) {
			bool is_temperatura = test_data[i].temperatura == a;
			attrb_temperatura_count[a] += (is_temperatura) ? 1 : 0;
			attrb_temperatura_count_per_class[a][is_temperatura && !(test_data[i].joga)] += is_temperatura;
		}
		for (u32 a = 0; a < UMIDADE_COUNT; ++a) {
			bool is_umidade = test_data[i].umidade == a;
			attrb_umidade_count[a] += (is_umidade) ? 1 : 0;
			attrb_umidade_count_per_class[a][is_umidade && !(test_data[i].joga)] += is_umidade;
		}
		for (u32 a = 0; a < VENTOSO_COUNT; ++a) {
			bool is_ventoso = test_data[i].ventoso == a;
			attrb_ventoso_count[a] += (is_ventoso) ? 1 : 0;
			attrb_ventoso_count_per_class[a][is_ventoso && !(test_data[i].joga)] += is_ventoso;
		}
	}

	// Calculate the Info(D) of the whole thing
	r32 info_d = 0.0f;
	for (u32 i = 0; i < class_number; ++i) {
		r32 p_i = (r32)class_count[i] / (r32)data_length;
		info_d += -(p_i * math_log(p_i) / math_log(2.0f));
	}

	// Calculate Info_Tempo(D)
	r32 info_d_tempo = 0.0f;
	for (u32 i = 0; i < TEMPO_COUNT; ++i) {
		r32 t = 0.0f;
		for (u32 j = 0; j < class_number; ++j) {
			r32 p_i = (r32)attrb_tempo_count_per_class[i][j] / (r32)attrb_tempo_count[i];
			if (p_i == 0) continue;
			t += -(p_i * math_log(p_i) / math_log(2.0f));
		}
		info_d_tempo += ((r32)attrb_tempo_count[i] / (r32)data_length) * t;
	}

	// Calculate Info_Temperatura(D)
	r32 info_d_temperatura = 0.0f;
	for (u32 i = 0; i < TEMPERATURA_COUNT; ++i) {
		r32 t = 0.0f;
		for (u32 j = 0; j < class_number; ++j) {
			r32 p_i = (r32)attrb_temperatura_count_per_class[i][j] / (r32)attrb_temperatura_count[i];
			if (p_i == 0) continue;
			t += -(p_i * math_log(p_i) / math_log(2.0f));
		}
		info_d_temperatura += ((r32)attrb_temperatura_count[i] / (r32)data_length) * t;
	}

	// Calculate Info_Umidade(D)
	r32 info_d_umidade = 0.0f;
	for (u32 i = 0; i < UMIDADE_COUNT; ++i) {
		r32 t = 0.0f;
		for (u32 j = 0; j < class_number; ++j) {
			r32 p_i = (r32)attrb_umidade_count_per_class[i][j] / (r32)attrb_umidade_count[i];
			if (p_i == 0) continue;
			t += -(p_i * math_log(p_i) / math_log(2.0f));
		}
		info_d_umidade += ((r32)attrb_umidade_count[i] / (r32)data_length) * t;
	}

	// Calculate Info_Tempo(D)
	r32 info_d_ventoso = 0.0f;
	for (u32 i = 0; i < VENTOSO_COUNT; ++i) {
		r32 t = 0.0f;
		for (u32 j = 0; j < class_number; ++j) {
			r32 p_i = (r32)attrb_ventoso_count_per_class[i][j] / (r32)attrb_ventoso_count[i];
			if (p_i == 0) continue;
			t += -(p_i * math_log(p_i) / math_log(2.0f));
		}
		info_d_ventoso += ((r32)attrb_ventoso_count[i] / (r32)data_length) * t;
	}

	r32 gain_tempo = info_d - info_d_tempo;
	r32 gain_temperatura = info_d - info_d_temperatura;
	r32 gain_umidade = info_d - info_d_umidade;
	r32 gain_ventoso = info_d - info_d_ventoso;
}

#endif

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

	// The number of different classes on the data set
	u32 num_classes;
};

void calculate_data_gains(File_Data* file_data, Data_Values* data_values);

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
	for (u32 i = 0; i < num_attribs; ++i) {
		attribute_aux_counter[i] = array_create(Attribute, 8);	// heuristic, expecting at most 8 different attribute values
	}

	for (s32 n = 0; n < num_entries; ++n) {
		for (s32 i = 0; i < num_attribs; ++i) {
			size_t attrib_type_count = array_get_length(attribute_aux_counter[i]);
			bool new_attrib = true;
			if (attrib_type_count > 0) {
				for (u32 k = 0; k < attrib_type_count; ++k) {
					if (attribute_equal(&file_data->attribs[num_attribs * n + i], &attribute_aux_counter[i][k])) {
						new_attrib = false;
						break;
					}
				}
			}
			if (new_attrib) {
				array_push(attribute_aux_counter[i], &file_data->attribs[num_attribs * n + i]);
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
	calculate_data_gains(file_data, &result);

	return result;
}

void calculate_data_gains(File_Data* file_data, Data_Values* data_values) {
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

	u32*** attrb_count_per_class = (u32***)calloc(1, file_data->num_attribs * max_attrib_value_count * class_number);


}