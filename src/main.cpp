#include "common.h"
#include "parser.h"
#include "util.h"
#include "id3.cpp"
#include "bootstrap.h"

struct Decision_Tree_Node {
	File_Data* node;
	Attribute class_of_node;		// only if pure, otherwise -1 (VALUE_TYPE_NONE)
	s32 attribute_index;
	s32 original_attribute_index;
	Decision_Tree_Node* children;	// array_release here
	Data_Values data_values;
};

void generate_decision_tree(Decision_Tree_Node* parent, File_Data* fdata, Data_Values* data_values = 0) 
{
	s32 attrib_index = -1;
	Data_Values values = extract_data_from_filedata(fdata, data_values);
	
	if (calculate_data_gains(fdata, &values, &attrib_index)) 
	{
		parent->data_values = values;
		parent->class_of_node.type = VALUE_TYPE_NONE;
		parent->attribute_index = attrib_index;
		parent->original_attribute_index += attrib_index;
		File_Data* branches = data_divide_on_attribute(&values, fdata);
		u32 num_branches = array_get_length(branches);
		parent->children = array_create(Decision_Tree_Node, num_branches);

		for (u32 i = 0; i < num_branches; ++i) 
		{
			size_t index = array_emplace(parent->children);
			parent->children[index].node = &branches[i];
			if(i >= attrib_index)
				parent->children[index].original_attribute_index = 1;
			else
				parent->children[index].original_attribute_index = 0;
			parent->children[index].attribute_index = 0;
			parent->children[index].children = 0;
			generate_decision_tree(&parent->children[i], &branches[i], &values);
		}

	} else {
		parent->class_of_node = fdata->attribs[fdata->class_index];
	}
}

Attribute decision_tree_get_class(Attribute* attribs, Decision_Tree_Node* tree) {
	u32 index = attribs[tree->original_attribute_index].value_int;
	Attribute result = {};

	if (tree->children[index].class_of_node.type != VALUE_TYPE_NONE) {
		result = tree->children[index].class_of_node;//attribs[tree->children[index].node->class_index];
	} else {
		Data_Values values = tree->data_values;
		result = decision_tree_get_class(attribs, &tree->children[index]);
	}

	return result;
}

void print_original_nodes(Decision_Tree_Node* node) {
	//printf("Node: %d\n", node->original_attribute_index);
	printf("Node: %d\n", node->class_of_node.type);
	if (!node->children)
		return;
	u32 num_child = array_get_length(node->children);
	for (int i = 0; i < num_child; ++i) {
		print_original_nodes(&node->children[i]);
	}
}

s32 main(s32 argc, s8** argv)
{
	File_Data fdata = parse_file((char*)"res/teste.data", 5, 4);

	const s32 num_bootstraps = 10;

	Decision_Tree_Node roots[num_bootstraps] = {};
	Bootstrap* bootstraps = bootstrap(fdata, num_bootstraps);

	print_bootstraps(bootstraps);

	for (s32 i = 0; i < num_bootstraps; ++i)
	{
		roots[i].node = &bootstraps[i].training_set;
		if (i == 4) {
			int x = 0;
		}
		generate_decision_tree(&roots[i], &bootstraps[i].training_set);
		for (s32 j = 0; j < bootstraps[i].training_set.num_entries; ++j)
		{
			Attribute result = decision_tree_get_class(bootstraps[i].training_set.attribs + j * bootstraps[i].training_set.num_attribs, &roots[i]);
			print("%d result = %d\n", j, result.value_int);
		}
	}
	//
	//for (u32 i = 0; i < fdata.num_entries; ++i) {
	//	Attribute result = decision_tree_get_class(&fdata.attribs[i * fdata.num_attribs], &root_node);
	//	printf("%d result = %d\n", i, result.value_int);
	//}

	return 0;
}
