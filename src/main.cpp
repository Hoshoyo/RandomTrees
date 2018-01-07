#include "common.h"
#include "parser.h"
#include "util.h"
#include "id3.cpp"

void generate_decision_tree(File_Data* fdata) {
	Data_Values values = extract_data_from_filedata(fdata);
	if (calculate_data_gains(fdata, &values)) {
		File_Data* branches = data_divide_on_attribute(&values, fdata);
		u32 num_branches = array_get_length(branches);
		for (u32 i = 0; i < num_branches; ++i) {
			Data_Values branches_values = extract_data_from_filedata(&branches[i]);
			if (calculate_data_gains(&branches[i], &branches_values)) {
				printf("\n");
				generate_decision_tree(&branches[i]);
			}
		}
	}
	printf("\n");
}

s32 main(s32 argc, s8** argv) 
{
	File_Data fdata = parse_file((char*)"res/teste.data", 5, 4);

	generate_decision_tree(&fdata);
	return 0;
}
