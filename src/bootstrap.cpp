#include "bootstrap.h"
#include "dynamic_array.h"
#include "util.h"

extern Bootstrap* bootstrap(File_Data original_file_data, s32 num_bootstraps)
{
	Bootstrap* bootstraps = array_create(Bootstrap, num_bootstraps);

	/* ALLOC MEMORY */

	bool** used_instances = array_create(bool*, num_bootstraps);

	for (s32 i = 0; i < num_bootstraps; ++i)
	{
		bool* instance_array = array_create(bool, original_file_data.num_entries);
		array_allocate(instance_array, original_file_data.num_entries);

		for (s32 j = 0; j < original_file_data.num_entries; ++j)
			instance_array[j] = false;

		array_push(used_instances, &instance_array);
	}

	/* ************* */

	for (s32 i = 0; i < num_bootstraps; ++i)
	{
		Bootstrap current_bootstrap = { 0 };
		File_Data current_training_set;

		current_training_set.num_entries = original_file_data.num_entries;
		current_training_set.integrity = original_file_data.integrity;
		current_training_set.class_index = original_file_data.class_index;
		current_training_set.num_attribs = original_file_data.num_attribs;
		current_training_set.value_types = original_file_data.value_types;
		current_training_set.types_indexes_remove_history = array_create(s32, original_file_data.num_attribs);
		for (s32 k = 0; k < original_file_data.num_attribs; ++k) {
			s32 minus_one = -1;
			array_push(current_training_set.types_indexes_remove_history, &minus_one);
		}
		current_training_set.num_max_attributes = original_file_data.num_attribs;
		current_training_set.tree_split_original_index = -1;
		current_training_set.attribs = array_create(Attribute, current_training_set.num_attribs * current_training_set.num_entries);

		for (s32 j = 0; j < original_file_data.num_entries; ++j)
		{
			s32 selected_instance = random_number() % original_file_data.num_entries;

			for (s32 k = selected_instance * original_file_data.num_attribs; k < selected_instance * original_file_data.num_attribs + original_file_data.num_attribs; ++k)
				array_push(current_training_set.attribs, &original_file_data.attribs[k]);

			used_instances[i][selected_instance] = true;
		}

		current_bootstrap.training_set = current_training_set;
		array_push(bootstraps, &current_bootstrap);
	}

	for (s32 i = 0; i < num_bootstraps; ++i)
	{
		File_Data current_test_set = { 0 };

		//current_test_set.num_entries = original_file_data.num_entries;
		current_test_set.integrity = original_file_data.integrity;
		current_test_set.class_index = original_file_data.class_index;
		current_test_set.num_attribs = original_file_data.num_attribs;
		current_test_set.value_types = original_file_data.value_types;
		current_test_set.attribs = array_create(Attribute, 1);

		for (s32 j = 0; j < original_file_data.num_entries; ++j)
		{
			if (!used_instances[i][j])
			{
				++current_test_set.num_entries;
				for (s32 k = j * original_file_data.num_attribs; k < j * original_file_data.num_attribs + original_file_data.num_attribs; ++k)
					array_push(current_test_set.attribs, &original_file_data.attribs[k]);
			}
		}

		bootstraps[i].test_set = current_test_set;
	}

	/* RELEASE MEMORY */

	for (s32 i = 0; i < array_get_length(used_instances); ++i)
		array_release(used_instances[i]);

	array_release(used_instances);

	/* ************** */
	return bootstraps;
}

extern void print_bootstraps(Bootstrap* bootstraps)
{
	for (s32 i = 0; i < array_get_length(bootstraps); ++i)
	{
		print("Bootstrap %d:\n", i);
		print("\tTraining Set:\n");

		for (s32 j = 0; j < bootstraps[i].training_set.num_entries; ++j)
		{
			print("\t\tInstance %d: {", j);

			for (s32 k = 0; k < bootstraps[i].training_set.num_attribs; ++k)
			{
				s32 initial_attrib_index = j * bootstraps[i].training_set.num_attribs + k;

				switch (bootstraps[i].training_set.value_types[k])
				{
				case VALUE_TYPE_INT: {
					print("%d", bootstraps[i].training_set.attribs[initial_attrib_index].value_int);
				} break;
				case VALUE_TYPE_BOOL: {
					if (bootstraps[i].training_set.attribs[initial_attrib_index].value_bool)
						print("true");
					else
						print("false");
				} break;
				case VALUE_TYPE_CHAR: {
					print("%c", bootstraps[i].training_set.attribs[initial_attrib_index].value_char);
				} break;
				case VALUE_TYPE_FLOAT: {
					print("%.3f", bootstraps[i].training_set.attribs[initial_attrib_index].value_float);
				} break;
				default: {
					print("ERROR");
				} break;
				}

				if (k != bootstraps[i].training_set.num_attribs - 1)
					print(", ");
			}

			print("}\n");
		}

		print("\Test Set:\n");

		for (s32 j = 0; j < bootstraps[i].test_set.num_entries; ++j)
		{
			print("\t\tInstance %d: {", j);

			for (s32 k = 0; k < bootstraps[i].test_set.num_attribs; ++k)
			{
				s32 initial_attrib_index = j * bootstraps[i].test_set.num_attribs + k;

				switch (bootstraps[i].test_set.value_types[k])
				{
				case VALUE_TYPE_INT: {
					print("%d", bootstraps[i].test_set.attribs[initial_attrib_index].value_int);
				} break;
				case VALUE_TYPE_BOOL: {
					if (bootstraps[i].test_set.attribs[initial_attrib_index].value_bool)
						print("true");
					else
						print("false");
				} break;
				case VALUE_TYPE_CHAR: {
					print("%c", bootstraps[i].test_set.attribs[initial_attrib_index].value_char);
				} break;
				case VALUE_TYPE_FLOAT: {
					print("%.3f", bootstraps[i].test_set.attribs[initial_attrib_index].value_float);
				} break;
				default: {
					print("ERROR");
				} break;
				}

				if (k != bootstraps[i].test_set.num_attribs - 1)
					print(", ");
			}

			print("}\n");
		}
	}
}