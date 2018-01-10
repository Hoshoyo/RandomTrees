#include "common.h"
#include "parser.h"
#include "util.h"
#include "id3.cpp"
#include "bootstrap.h"

// Structure representing one Decision Tree
struct Decision_Tree_Node 
{
	File_Data* node;				// Data
	Attribute class_of_node;		// only if pure, otherwise -1 (VALUE_TYPE_NONE)
	Decision_Tree_Node* children;	// array_release here

	Data_Values data_values;		// internal use
};

// Generate the decision tree given a File_Data
// - parent			: (out) the structure where the root node will be stored
// - fdata			: (in)  the data loaded for each bootstrap
// - data_values	: should be 0 only used internally for recursion
//
// Return value
// boolean on the purity of the node (true if pure false otherwise)
bool generate_decision_tree(Decision_Tree_Node* parent, File_Data* fdata, Data_Values* data_values = 0) 
{
	s32 attrib_index = -1;
	Data_Values values = extract_data_from_filedata(fdata, data_values);
	
	// if the num_entries is 0 there is no data representing this node, class 0 for default as guess
	// @todo i dont know what to do here
	if (fdata->num_entries == 0) {
		parent->class_of_node.type = VALUE_TYPE_INT;
		parent->class_of_node.value_int = 0;
		return true;
	}

	// Calculate the attribute that gives the maximum gain put it on data_values.biggest_gain_index and attrib_index
	if (calculate_data_gains(fdata, &values, &attrib_index)) 
	{
		parent->data_values = values;
		parent->class_of_node.type = VALUE_TYPE_NONE;
		File_Data* branches = data_divide_on_attribute(&values, fdata);
		u32 num_branches = array_get_length(branches);
		parent->children = array_create(Decision_Tree_Node, num_branches);

		for (u32 i = 0; i < num_branches; ++i) 
		{
			size_t index = array_emplace(parent->children);
			parent->children[index].node = &branches[i];
			parent->children[index].children = 0;
			s32* backup = (s32*)malloc(parent->node->num_max_attributes * sizeof(s32));
			memcpy(backup, parent->node->types_indexes_remove_history, parent->node->num_max_attributes * sizeof(s32));
			generate_decision_tree(&parent->children[i], &branches[i], &values);
			memcpy(parent->node->types_indexes_remove_history, backup, parent->node->num_max_attributes * sizeof(s32));
			free(backup);
		}

	} else {
		// Node is pure, fill the class_of_node in the parent node
		parent->class_of_node = fdata->attribs[fdata->class_index];
		return true;
	}
	return false;
}

// Get the predicted class given a decision tree
// Class is returned as an Attribute where value_int is the index of it
Attribute decision_tree_get_class(Attribute* attribs, Decision_Tree_Node* tree) 
{
	u32 index = attribs[tree->node->tree_split_original_index].value_int;
	Attribute result = {};

	if (tree->children[index].class_of_node.type != VALUE_TYPE_NONE) {
		result = tree->children[index].class_of_node;
	} else {
		result = decision_tree_get_class(attribs, &tree->children[index]);
	}

	return result;
}

s32 main(s32 argc, s8** argv)
{
	File_Data fdata = parse_file((char*)"res/teste2.data", 5, 4);
	//File_Data fdata = parse_file((char*)"res/haberman.data", 4, 3);
	//File_Data fdata = parse_file((char*)"res/cmc.data", 10, 9);

	const s32 num_bootstraps = 10;

	Decision_Tree_Node roots[num_bootstraps] = {};
	// Generate bootstraps
	Bootstrap* bootstraps = bootstrap(fdata, num_bootstraps);
	
	// Generate the decision tree
	for (s32 i = 0; i < num_bootstraps; ++i)
	{
		roots[i].node = &bootstraps[i].training_set;
		generate_decision_tree(&roots[i], &bootstraps[i].training_set);
	}

	// Run test for each testing set of each bootstrap
	for (s32 test_boots_index = 0; test_boots_index < num_bootstraps; ++test_boots_index) {

		s32 count_success = 0;
		s32 count_failure = 0;

		s32 num_classes = fdata.count_value_types_each_attribute[fdata.class_index];

		// hold count for later voting
		s32* class_count = (s32*)calloc(num_classes, bootstraps[test_boots_index].test_set.num_entries * sizeof(s32));

		// Generate the random trees 1 for each bootstrap
		for (s32 i = 0; i < num_bootstraps; ++i)
		{
			for (s32 j = 0; j < bootstraps[test_boots_index].test_set.num_entries; ++j) {
				Attribute result = decision_tree_get_class(bootstraps[test_boots_index].test_set.attribs + j * bootstraps[test_boots_index].test_set.num_attribs, &roots[i]);
				Attribute expected_result = bootstraps[test_boots_index].test_set.attribs[j * bootstraps[test_boots_index].test_set.num_attribs + bootstraps[test_boots_index].test_set.class_index];
				class_count[j * num_classes + result.value_int] += 1;
			}
		}

		// check success and failure rating
		for (s32 i = 0; i < bootstraps[test_boots_index].test_set.num_entries; ++i) {
			// pick majority (voting)
			s32 max = -1;
			s32 max_index = -1;
			for (s32 c = 0; c < num_classes; ++c) {
				if (class_count[i * num_classes + c] > max) {
					max = class_count[i * num_classes + c];
					max_index = c;
				}
			}
			if (max_index == bootstraps[test_boots_index].test_set.attribs[i * bootstraps[test_boots_index].test_set.num_attribs + bootstraps[test_boots_index].test_set.class_index].value_int) {
				count_success += 1;
			} else {
				count_failure += 1;
			}
		}
		free(class_count);
		printf("Success: %d\nFailure: %d\n\n", count_success, count_failure);
	}
	return 0;
}
