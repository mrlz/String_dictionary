#include "linear_hash.hpp"

//Hash_Table constructor, in order to make it more competitive to the trees we start with a small
//size of 100. Every time the load factor grows beyond 0.4 we double the table size.
//A table is made of hash_slots, which contain a string key and a vector of vectors to store the values
//for each text.
//We keep a different counter for stored_elements (which counts the number of hash_slots being used) and
//inserted (which counts the number of insertions performed), because a single hash_slot could store all
//inserted values if they all corresponded to the same key, this comes in handy to compute the total size.
Hash_Table::Hash_Table(){ //since each element is a pointer to a char* array we pay 4 bytes per item.
  this->table_size = 100;
  this->stored_elements = 0;
  this->hash_table = new hash_slot[this->table_size + 1];
  this->hash_table[this->table_size].key = "VALUE NOT FOUND";
  this->inserted = 0;
}

//Returns the value of the hash function in modulo table_size;
std::size_t Hash_Table::hash_value(std::string pattern){
  return this->hash_function(pattern)%this->table_size;
}

//Returns the load factor of the table.
double Hash_Table::get_fill(){
  return this->stored_elements/(double)this->table_size;
}

//Probes the table to find a free slot, it starts at the hash_value given by the
//hash function and then advances in linear fashion, wrapping around the borders
//of the table. Since the load factor is kept <= 0.4 this will always find a slot.
std::size_t Hash_Table::probe_free_slot(std::size_t hash_value, hash_slot *hash_table_to_probe, std::string pattern){
  std::size_t slot = hash_value;
  while(hash_table_to_probe[slot].key != ""){
    if (hash_table_to_probe[slot].key == pattern){
      break;
    }
    slot = (slot+1)%table_size;
  }
  return slot;
}

//In case we need to expand the table this function rehashes the old hash_slots
//to the new table.
void Hash_Table::rehash_entries(hash_slot *old_table, hash_slot *new_table){
  std::size_t original_size = this->table_size/2;
  new_table[this->table_size].key = "VALUE NOT FOUND";
  for(std::size_t i = 0; i < original_size ; i++ ){
    if(old_table[i].key != ""){
      new_table[this->probe_free_slot(this->hash_value(old_table[i].key), new_table, old_table[i].key)] = old_table[i];
    }
  }
}

//This function checks the load factor, and if the threshold has been surpassed then
//it doubles the table and rehashes the elements.
void Hash_Table::check_fill_rate(){
  if ((( this->stored_elements + 1)/(double)this->table_size) > 0.4){
    this->table_size = 2*this->table_size;
    hash_slot *new_table = new hash_slot[this->table_size + 1];
    this->rehash_entries(this->hash_table, new_table);
    delete[] this->hash_table;
    this->hash_table = new_table;
  }
}

//Checks whether the insertion to a hash_slot is the first, for accounting purposes.
bool Hash_Table::first_insertion(std::size_t slot){
  return (this->hash_table[slot].text_position.size() == 0);
}

//Insertion function, augments the inserted value by one and the stored value by one if
//the pattern was not previously found in the table. It also checks that the load factor
//be within bounds. If it is a first insertion then the hash_slot must be given
//the vectors to store the values per text, if it is not a first insertion then the value
//is just added to the corresponding vector.
void Hash_Table::insert(std::string pattern, std::size_t value, int text_index){
  this->inserted++;
  this->check_fill_rate();
  std::size_t slot = this->probe_free_slot(this->hash_value(pattern), this->hash_table, pattern);
  if(this->first_insertion(slot)){
    this->hash_table[slot].text_position.push_back(std::vector<std::size_t> {});
    this->hash_table[slot].text_position.push_back(std::vector<std::size_t> {});
    this->stored_elements++;
  }
  this->hash_table[slot].key = pattern;
  this->hash_table[slot].text_position[text_index].push_back(value);
}

//Deletes the hash_table array.
void Hash_Table::delete_data(){
  delete[] this->hash_table;
}

//Searches for the pattern in the table using the hash value of the hash function
//as the first spot to look, then inspects linearly.
std::size_t Hash_Table::search(std::string pattern){
  std::size_t start_position = this->hash_value(pattern);
  while(this->hash_table[start_position].key != ""){
    if(this->hash_table[start_position].key == pattern){
      return start_position;
    }
    start_position = (start_position+1)%this->table_size;
  }
  return this->table_size;
}

//Returns the key stored at a specified slot.
std::string Hash_Table::key_in_slot(std::size_t slot){
  return this->hash_table[slot].key;
}

//Prints all the values stored at a given slot, for a given text.
void Hash_Table::print_positions(std::size_t slot, int text_index){
  for(int i = 0; i < this->hash_table[slot].text_position[text_index].size(); i++){
    std::cout << hash_table[slot].text_position[text_index][i] << ", ";
  }
}

//Reports whether the pattern is in the table.
//If print is enabled prints information pertaining to the values associated
//to the pattern in the given text.
bool Hash_Table::search_report(std::string pattern, int text_index, int print){
  std::size_t slot = this->search(pattern);
  if (print){
  std::cout << pattern << " found in slot " << slot << " with " << this->hash_table[slot].text_position[text_index].size() << " occurences: ";
  this->print_positions(slot, text_index);
  std::cout << std::endl;
  }
  if (pattern == this->hash_table[slot].key){
    return 1;
  }
  return 0;
}

//Computes the total size used by a slot.
std::size_t Hash_Table::slot_size(struct hash_slot slot){
  std::size_t size = sizeof(struct hash_slot);
  if (slot.key != ""){
    size = size + slot.key.capacity();
    size = size + slot.text_position.capacity()*sizeof(std::vector<std::size_t>);
    // std::cout << "accesing arrays " << std::endl;
    size = size + (slot.text_position[0].capacity() + slot.text_position[1].capacity())*sizeof(std::size_t);
    // std::cout << "done accesing" << std::endl;
  }
  return size;
}

//Returns the size of the structure, in bytes.
std::size_t Hash_Table::structure_size(){
  //We have to account for the node that will not be checked.
  std::size_t hash_slot_info = sizeof(Hash_Table) + this->name.capacity()*sizeof(char) + this->hash_table[this->table_size].key.capacity()*sizeof(std::size_t) + sizeof(struct hash_slot);
  for(int i = 0; i < this->table_size; i++){
    hash_slot_info = hash_slot_info + this->slot_size(this->hash_table[i]);
  }
  return hash_slot_info;
}

//Returns all occurences of the pattern, for each text.
std::vector<std::size_t> Hash_Table::occurences(std::string pattern){
  std::size_t slot = this->search(pattern);
  return std::vector<std::size_t> {this->hash_table[slot].text_position[0].size(), this->hash_table[slot].text_position[1].size()};
}

//The following are simple examples of usage.
void example_1_hash(){
  Hash_Table *Table = new Hash_Table();
  Table->insert("SOME", 0, 0);
  Table->insert("ABACUS", 4, 0);
  Table->insert("SOMETHING", 16, 0);
  Table->insert("B", 50, 0);
  Table->insert("ABRACADABRA", 40, 0);
  Table->insert("THIS", 10, 0);
  Table->insert("SOMERSET", 11, 0);

  Table->search_report("SOME",0, 1);
  Table->search_report("ABACUS",0, 1);
  Table->search_report("SOMETHING",0, 1);
  Table->search_report("B",0, 1);
  Table->search_report("ABRACADABRA",0, 1);
  Table->search_report("THIS",0, 1);
  Table->search_report("SOMERSET",0, 1);

  Table->delete_data();
  delete Table;
}

void example_2_hash(){
  Hash_Table* Table = new Hash_Table;
  Table->insert("is", 10, 0);
  Table->insert("is",0,0);

  Table->insert("in", 20, 0);
  Table->insert("in",3,0);
  Table->insert("in",6,0);

  Table->insert("it", 220, 0);
  Table->insert("it",9,0);

  Table->insert("be", 1230, 0);
  Table->insert("be",12,0);
  Table->insert("be",15,0);
  Table->insert("be",18,0);

  Table->insert("by", 1230, 0);
  Table->insert("by",21,0);
  Table->insert("by",24,0);

  Table->insert("on", 123120, 0);
  Table->insert("on",27,0);
  Table->insert("on",30,0);
  Table->insert("on",33,0);
  Table->insert("on",36,0);
  Table->insert("on",39,0);

  Table->insert("of", 44230, 0);
  Table->insert("of",42,0);
  Table->insert("of",45,0);

  Table->insert("or", 1230, 0);
  Table->insert("or",48,0);
  Table->insert("or",51,0);

  Table->insert("as", 41240, 0);
  Table->insert("as",54,0);

  Table->insert("at", 111230, 0);
  Table->insert("at",57,0);

  Table->insert("he", 33210, 0);
  Table->insert("he",60,0);

  Table->insert("to", 11220, 0);
  Table->insert("to",63,0);
  Table->insert("to",66,0);
  Table->insert("to",69,0);
  Table->insert("to",72,0);
  Table->insert("to",75,0);

  Table->search_report("is",0, 1);
  Table->search_report("in",0, 1);
  Table->search_report("it",0, 1);
  Table->search_report("be",0, 1);
  Table->search_report("by",0, 1);
  Table->search_report("on",0, 1);
  Table->search_report("of",0, 1);
  Table->search_report("or",0, 1);
  Table->search_report("as",0, 1);
  Table->search_report("at",0, 1);
  Table->search_report("he",0, 1);
  Table->search_report("to",0, 1);

  Table->delete_data();
  delete Table;
}

//UNCOMMENT MAIN TO TEST EXAMPLES IN STANDALONE FASHION
// int main(){
//   example_1_hash();
//   return 0;
// }
