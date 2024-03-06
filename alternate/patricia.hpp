#ifndef __P_H_INCLUDED__
#define __P_H_INCLUDED__

#include <boost/dynamic_bitset.hpp>
#include <bitset>
#include <string>
#include <iostream>
#include <algorithm>

//This struct is used to print the tree
struct cell_display {
    std::string   valstr;
    bool     present;
    cell_display() : present(false) {}
    cell_display(std::string valstr) : valstr(valstr), present(true) {}
};

//This is the core of the patricia tree, its node.
//Has pointers to both childs (since it's a binary tree) and a third leaf pointer
//with which an inner node can point to any of its leaves, for quick access.
//left_offset and right_offset serve as jump values for the branches. Once a bit
//of the pattern has been inspected in the node we jump left_offset or right_offset
//units in the pattern and advance to the corresponding branch.
//text_position contains the values associated with the key for each text.
//In the case of leaf nodes, which don't branch, left_offset is used as start_position
//and right_offset is used as offset, to encode a pattern in the dictionary.
struct node{
  struct node *left_child;
  int left_offset;
  struct node *right_child;
  int right_offset;
  struct node *leaf;
  std::vector<std::vector<std::size_t>> text_position;
};

//The Patricia_Tree class declaration. Details which methods and arguments
//are public and private.
class Patricia_Tree{
public:
  Patricia_Tree();
  void insert(std::string pattern, std::size_t value, int text_index);
  // void insert(boost::dynamic_bitset<> bit_pattern, std::size_t value, int text_index);
  void delete_data();
  struct node* search(std::string pattern);
  void print_tree(int type);
  bool search_report(std::string pattern, int text_index, int print);
  std::string get_name(){return this->name;}
  std::size_t structure_size();
  double extra_measurement(){return this->find_depth(this->root);}
  std::size_t occurences(std::string pattern, int text_index);
  void print_dictionary();
private:
  struct node *root = new struct node();
  // boost::dynamic_bitset<> dictionary;
  std::string dictionary;
  std::string name = "PATR";

  int find_max_prefix(std::string pattern, int start_pos, int offset);
  bool is_leaf_the_pattern(struct node* leaf, std::string pattern);
  struct node *node_search(std::string pattern, int end_point, int *final_slot, struct node* parent, int *child_type);
  int insert_word_to_dictionary(std::string pattern);
  struct node *insert_word_and_make_leaf(std::string pattern, std::size_t value, int text_index);
  struct node *split_leaf(struct node* parent, struct node* candidate, struct node* new_pattern_node, bool branch, bool child_type);
  void insert_missing_son(struct node* candidate, struct node* new_pattern_node, bool branch);
  void split_arc(struct node* parent, struct node* candidate, struct node* new_pattern_node, bool branch, bool child_type, int difference_of_offset);
  void extend_arc_and_split_leaf(struct node* parent, struct node* candidate, struct node* new_pattern_node, bool branch, bool child_type, int difference_of_offset);
  void reinsert_from_leaf(struct node* leaf, std::string pattern, std::size_t value, int text_index, int first_different_position);
  void delete_node(struct node* node);
  int find_depth(struct node* node);
  std::vector<std::vector<cell_display>> get_row_display(int type);
  std::vector<std::string> row_formatter(const std::vector<std::vector<cell_display>>& rows_disp);
  void trim_rows_left(std::vector<std::string>& rows);
  void print_positions(struct node* node, int text_index);
  std::size_t pattern_occurences_cost(std::vector<std::vector<std::size_t>> occurences);
  std::size_t node_cost(struct node* node);
};

boost::dynamic_bitset<> word_to_bitset(std::string word);

#endif
