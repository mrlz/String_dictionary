#ifndef __T_H_INCLUDED__
#define __T_H_INCLUDED__

#include <string>
#include <iostream>
#include <algorithm>

//At the core of a ternary search tree is a ternary node:
//- 3 pointers to children
//- 1 string to store the character (could be a char, which would save us some good 7 bits, but it'd imply a casting)
//- 1 vector of vectors to hold the values for each text
//- 1 boolean that indicates whether the node acts as a lesser_character node
struct ternary_node{
  struct ternary_node* lesser_child;
  struct ternary_node* equal_child;
  struct ternary_node* greater_child;
  std::string character;
  std::vector<std::vector<std::size_t>> text_position;
  bool counts_as_lesser;
};

//Ternary_Search_Tree class, indicates which methods are public and which are private.
class Ternary_Search_Tree{
public:
  Ternary_Search_Tree();
  void insert(std::string pattern, std::size_t value, int text_index);
  struct ternary_node* search(std::string pattern);
  void delete_data();
  void print_tree();
  bool search_report(std::string pattern, int text_index, int print);
  std::string get_name(){return this->name;}
  std::size_t structure_size();
  double extra_measurement(){return this->find_depth(this->root);}
  std::vector<std::size_t> occurences(std::string pattern);
private:
  struct ternary_node *root = new struct ternary_node();
  std::string name = "TERN";

  void delete_node(struct ternary_node* node);
  void insert_to_node(std::string pattern, struct ternary_node** node, int text_index, std::size_t value);
  struct ternary_node* node_search(std::string pattern, struct ternary_node* node);
  int find_depth(struct ternary_node* node);
  void print_node_contents(struct ternary_node* node, std::string offset, int depth);
  struct ternary_node* new_node(std::string character);
  std::string lesser_character = std::string(1,(char)1);
  void print_positions(struct ternary_node* node, int text_index);
  std::size_t pattern_occurences_cost(std::vector<std::vector<std::size_t>> occurences);
  std::size_t node_cost(struct ternary_node* node);
};

#endif
