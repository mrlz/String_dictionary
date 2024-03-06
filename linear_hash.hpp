#ifndef __L_H_INCLUDED__
#define __L_H_INCLUDED__

#include <string>
#include <iostream>
#include <functional>
#include <vector>

//A Hash_Table is, mainly, an array of hash_slots.
struct hash_slot{
  std::string key;
  std::vector<std::vector<std::size_t>> text_position;
};

//Hash_Table class declaration, lists public and private methods.
class Hash_Table{
public:
  Hash_Table();
  void insert(std::string, std::size_t value, int text_index);
  std::size_t search(std::string);
  void delete_data();
  std::string key_in_slot(std::size_t slot);
  bool search_report(std::string pattern, int text_index, int print);
  std::string get_name(){return this->name;}
  std::size_t structure_size();
  double get_fill();
  double extra_measurement(){return this->get_fill();}
  std::vector<std::size_t> occurences(std::string pattern);
private:
  hash_slot *hash_table;
  std::size_t table_size;
  std::size_t stored_elements;
  std::string name = "HASH";
  std::size_t inserted;

  std::hash<std::string> hash_function;
  std::size_t hash_value(std::string pattern);
  std::size_t probe_free_slot(std::size_t hash_value, hash_slot *hash_table_to_probe, std::string pattern);
  void rehash_entries(hash_slot *old_table, hash_slot *new_table);
  bool first_insertion(std::size_t slot);
  void check_fill_rate();
  std::size_t slot_size(struct hash_slot slot);
  void print_positions(std::size_t slot, int text_index);
};

#endif
