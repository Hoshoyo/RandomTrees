#include "common.h"
#include "parser.h"

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

typedef bool Class_Type;

r32 math_log(r32 n) {
	return logf(n);
}

static void print_min_and_max(File_Data& fdata){
	for(int i = 0; i < fdata.num_attribs; ++i){
		switch(fdata.min_attribs[i].type){
			case VALUE_TYPE_INT:{
				printf("int   attrib %d min: %d, max: %d\n", i, fdata.min_attribs[i].value_int, fdata.max_attribs[i].value_int);
			}break;
			case VALUE_TYPE_FLOAT:{	
				printf("float attrib %d min: %.02f, max: %.02f\n", i, fdata.min_attribs[i].value_float, fdata.max_attribs[i].value_float);
			}break;
			case VALUE_TYPE_CHAR:{
				printf("char  attrib %d min: %c, max: %c\n", i, fdata.min_attribs[i].value_char, fdata.max_attribs[i].value_char);
			}break;
		}
	}
}

s32 main(s32 argc, s8** argv) 
{
	//if(argc > 1) {
		File_Data fdata = parse_file((char*)"res/cmc.data", 10);
		print_min_and_max(fdata);
	//}

	const u32 data_length = sizeof(test_data) / sizeof(Data);
	const u32 class_number = 2;
	const u32 attrib_number = 4;
	Class_Type classes[class_number] = {true, false};

	// Calculate counts for the data set
	u32 class_count[class_number] = { };
	for (u32 i = 0; i < class_number; ++i)
		class_count[i] = data_class_count(test_data, data_length, classes[i]);
	
	u32 attrb_tempo_count[TEMPO_COUNT] = { };
	u32 attrb_temperatura_count[TEMPERATURA_COUNT] = { };
	u32 attrb_umidade_count[UMIDADE_COUNT] = { };
	u32 attrb_ventoso_count[VENTOSO_COUNT] = { };

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
	r32 info_d_umidade= 0.0f;
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

	return 0;
}
