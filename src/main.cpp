#include "common.h"
#include "parser.h"
#include "util.h"
#include "id3.cpp"

s32 main(s32 argc, s8** argv) 
{
	File_Data fdata = parse_file((char*)"res/teste.data", 5, 4);
	Data_Values values = extract_data_from_filedata(&fdata);


	return 0;
}
