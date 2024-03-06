#include "ternary.hpp"

//Class constructor, we merely need to initialize the root as nullptr;
Ternary_Search_Tree::Ternary_Search_Tree(){
  this->root = nullptr;
}

//This method creates a node to store the character. All the fields are declared
//nullptr because the insertion function first creates a node and then fills it
//up with the corresponding information.
//We add the char 1 to all patterns before inserting them, which is lexicographically
//smaller than any char used in the texts or the randomly generated strings.
//If the node is to hold this pattern, refered to henceforth as lesser_character,
//then it will need text_position to contain the corresponding vectors to each text,
//so we can store the values associated with the key.
//The counts_as_lesser field is necessary because it could be the case that we
//would want to insert a node containing the lesser_character at a point where
//there is already another node (e.g: Say we inserted the pattern re + lesser character,
//we would have a node with 'r', its equal child would be 'e', and 'e' would have
//an equal child 'lesser_character'. If we, then, inserted the pattern r + lesser_character)
//we would probe the tree, go to the node with r, advance to its equal child, where
//we would want to insert the lesser_pattern node, but e is already there, so we need
//to equip nodes with the plasticity to act as lesser_character nodes simultaneously.
struct ternary_node* Ternary_Search_Tree::new_node(std::string character){
  struct ternary_node* new_node = new struct ternary_node();
  new_node->lesser_child = nullptr;
  new_node->equal_child = nullptr;
  new_node->greater_child = nullptr;
  new_node->character = character;
  new_node->counts_as_lesser = 0;
  if(character == this->lesser_character){
    new_node->counts_as_lesser = 1;
    new_node->text_position.push_back(std::vector<std::size_t> {});
    new_node->text_position.push_back(std::vector<std::size_t> {});
  }
  return new_node;
}

//This method probes a node to see if its character corresponds to the first character
//of the pattern. If it is and the only remaining character is lesser_character, we
//check whether the equal child exists to create it if necessary, or if it acts as
//a lesser_character, in which case we just add the value. If the node exists, but
//is not acting as lesser_character (because it holds some other character) we enable
//it to act as a lesser_character node and add the value after creating the text vectors.
//In case more of the pattern remains, we insert the pattern, without the first character,
//to the equal_child.

//If the character is greater or lesser we insert the whole pattern again to the greater_child
//or lesser_child, respectively.

//Note that when we insert the pattern to a child it might be the case that the child
//is nullptr. If this is the case we create the node and reinsert the pattern to the new node.
void Ternary_Search_Tree::insert_to_node(std::string pattern, struct ternary_node** node, int text_index, std::size_t value){
  if ((*node) == nullptr){
    (*node) = this->new_node(pattern.substr(0,1));
    this->insert_to_node(pattern, node, text_index, value);
    return;
  }
  if (pattern.substr(0,1) == (*node)->character){
    std::string remaining_characters = pattern.substr(1,pattern.size()-1);
    if (remaining_characters == this->lesser_character){

      if(((*node)->equal_child) == nullptr){
        (*node)->equal_child = this->new_node(remaining_characters);
      }
      if((*node)->equal_child->counts_as_lesser){
      (*node)->equal_child->text_position[text_index].push_back(value);
      }else{
        (*node)->equal_child->counts_as_lesser = 1;
        (*node)->equal_child->text_position.push_back(std::vector<std::size_t> {});
        (*node)->equal_child->text_position.push_back(std::vector<std::size_t> {});
        (*node)->equal_child->text_position[text_index].push_back(value);
      }
      return;
    }else{
      this->insert_to_node(remaining_characters, &((*node)->equal_child), text_index, value);
    }
  }else if(pattern.substr(0,1) < (*node)->character){
    this->insert_to_node(pattern, &((*node)->lesser_child), text_index, value);
  }else{
    this->insert_to_node(pattern, &((*node)->greater_child), text_index, value);
  }
}

//Insertion method that checks whether the root is nullptr and creates it if necessary,
//if it is not nullptr then uses the previous method to insert from the root.
void Ternary_Search_Tree::insert(std::string pattern, std::size_t value, int text_index){
  pattern = pattern + this->lesser_character;
  if (this->root == nullptr && pattern != this->lesser_character){
    *(&(this->root)) = this->new_node(pattern.substr(0,1));
    this->insert_to_node(pattern,&(this->root), text_index, value);
    return;
  }
  this->insert_to_node(pattern, &(this->root), text_index, value);
}

//Deletes all the nodes in the tree.
void Ternary_Search_Tree::delete_node(struct ternary_node* node){
  if (node != nullptr){
  this->delete_node(node->lesser_child);
  this->delete_node(node->equal_child);
  this->delete_node(node->greater_child);
  delete node;
  }
}

//Common interface of the 3 structures
void Ternary_Search_Tree::delete_data(){
  this->delete_node(this->root);
}

//Searches for a pattern in the given node. Navigates the tree just like insertion does.
struct ternary_node* Ternary_Search_Tree::node_search(std::string pattern, struct ternary_node* node){

  if (node == nullptr){
    return nullptr;
  }

  if (node->character == pattern.substr(0,1)){
    if (pattern.size() == 2 && node->equal_child->counts_as_lesser){
      return node->equal_child;
    }
    return this->node_search(pattern.substr(1,pattern.size()-1), node->equal_child);
  }else if(node->character > pattern.substr(0,1)){
    return this->node_search(pattern, node->lesser_child);
  }else{
    return this->node_search(pattern, node->greater_child);
  }

}

//Searches the pattern in the tree from the root.
struct ternary_node* Ternary_Search_Tree::search(std::string pattern){
  return this->node_search(pattern + this->lesser_character, this->root);
}

//Finds the depth of the tree rooted at node.
int Ternary_Search_Tree::find_depth(struct ternary_node* node){
  if(node != nullptr){
    int lesser = this->find_depth(node->lesser_child);
    int equal = this->find_depth(node->equal_child);
    int greater = this->find_depth(node->greater_child);
    return 1 + std::max(std::max(lesser,greater),equal);
  }
  return 0;
}

//Prints the contents of a node.
void Ternary_Search_Tree::print_node_contents(struct ternary_node* node, std::string offset, int depth){
  if (depth == 0){
    if(node != nullptr){
      std::cout << offset << node->character;
    }else{
      std::cout << offset << "∅";
    }
  }else if (node != nullptr){
    this->print_node_contents(node->lesser_child, offset, depth-1);
    this->print_node_contents(node->equal_child, offset, depth-1);
    this->print_node_contents(node->greater_child, offset, depth-1);
    std::cout << " |";
  }else{
    std::cout << offset << "∅"<< offset << "∅" << offset << "∅ |";
  }
}

//Prints a tree level by level. Not as pretty as the method for Patricia_Tree, but
//less involved.
void Ternary_Search_Tree::print_tree(){
  std::string offset = "                                                        ";
  struct ternary_node *current = this->root;
  int depth = this->find_depth(this->root);
  std::cout << "printing tree of depth " << depth << std::endl;
  for(int d = 0; d < depth; d++){
    std::cout << "depth " << d << std::endl;
    this->print_node_contents(current, offset, d);
    std::cout << std::endl;
    offset = offset.substr(0,offset.size()/2 -2);
  }
}

//Prints the values associated with the key that the node represents, for a given text.
void Ternary_Search_Tree::print_positions(struct ternary_node* node, int text_index){
  for(int i = 0; i < node->text_position[text_index].size(); i++){
    std::cout << node->text_position[text_index][i] << ", ";
  }
}

//Performs a search in the Tree for the pattern and reports if it finds it or not.
//If print is enabled it gives the values associated with the key for a given text.
bool Ternary_Search_Tree::search_report(std::string pattern, int text_index, int print){
  struct ternary_node* s = this->search(pattern);
  if (s != nullptr){
    if (print){
    std::cout << pattern << " has been found >> ";
    std::cout << "according to the node it has " << s->text_position[text_index].size() << " ocurrences at slots ";
    this->print_positions(s,text_index);
    std::cout << std::endl;
    }
    return 1;
  }
  return 0;
}

//Returns the total size spent by the node to store the vectors of values.
std::size_t Ternary_Search_Tree::pattern_occurences_cost(std::vector<std::vector<std::size_t>> occurences){
  if(occurences.size() > 0){
    return sizeof(std::size_t)*(occurences[0].capacity() + occurences[1].capacity());
  }
  return 0;
}

//Computes the cost of the subtree rooted at node.
std::size_t Ternary_Search_Tree::node_cost(struct ternary_node* node){
  if(node != nullptr){
    std::size_t lesser = this->node_cost(node->lesser_child);
    std::size_t equal = this->node_cost(node->equal_child);
    std::size_t greater = this->node_cost(node->greater_child);
    return node->text_position.capacity()*sizeof(std::vector<std::size_t>) + sizeof(struct ternary_node) + lesser + equal + greater + this->pattern_occurences_cost(node->text_position) + node->character.capacity()*sizeof(char); // Node size is 57 bytes, but 64 due to padding.
  }
  return 0;
}

//Returns the total structure size in bytes.
std::size_t Ternary_Search_Tree::structure_size(){
  std::size_t base = sizeof(Ternary_Search_Tree) + this->name.capacity()*sizeof(char);
  if(this->root != nullptr){
    return this->node_cost(this->root) + base;
  }
  return base;
}

//Returns all occurences of the pattern, for each text.
std::vector<std::size_t> Ternary_Search_Tree::occurences(std::string pattern){
  struct ternary_node* s = this->search(pattern);
  return std::vector<std::size_t> {s->text_position[0].size(), s->text_position[1].size()};
}

//Simple, plainly laid out, example of Tree usage.
void example_1_ternary(){
  std::cout << "Creating tree" << std::endl;
  Ternary_Search_Tree *Tree = new Ternary_Search_Tree();
  std::cout << "Inserting words from text: is in it be by on of or as at he to" << std::endl;

  Tree->insert("is",0,0);

  Tree->insert("in",3,0);
  Tree->insert("in",6,0);

  Tree->insert("it",9,0);

  Tree->insert("be",12,0);
  Tree->insert("be",15,0);
  Tree->insert("be",18,0);


  Tree->insert("by",21,0);
  Tree->insert("by",24,0);

  Tree->insert("on",27,0);
  Tree->insert("on",30,0);
  Tree->insert("on",33,0);
  Tree->insert("on",36,0);
  Tree->insert("on",39,0);

  Tree->insert("of",42,0);
  Tree->insert("of",45,0);

  Tree->insert("or",48,0);
  Tree->insert("or",51,0);

  Tree->insert("as",54,0);

  Tree->insert("at",57,0);

  Tree->insert("he",60,0);

  Tree->insert("to",63,0);
  Tree->insert("to",66,0);
  Tree->insert("to",69,0);
  Tree->insert("to",72,0);
  Tree->insert("to",75,0);




  std::cout << "Done inserting" << std::endl;
  Tree->print_tree();

  Tree->search_report("is", 0, 1);
  Tree->search_report("in", 0, 1);
  Tree->search_report("it", 0, 1);
  Tree->search_report("be", 0, 1);
  Tree->search_report("by", 0, 1);
  Tree->search_report("on", 0, 1);
  Tree->search_report("of", 0, 1);
  Tree->search_report("or", 0, 1);
  Tree->search_report("as", 0, 1);
  Tree->search_report("at", 0, 1);
  Tree->search_report("he", 0, 1);
  Tree->search_report("to", 0, 1);

  if ( Tree->search("is") != nullptr && Tree->search("in") != nullptr && Tree->search("it") != nullptr && Tree->search("be") != nullptr && Tree->search("by") != nullptr && Tree->search("on") != nullptr && Tree->search("of") != nullptr && Tree->search("or") != nullptr && Tree->search("as") != nullptr && Tree->search("at") != nullptr && Tree->search("he") != nullptr && Tree->search("to") != nullptr ){
    std::cout << "all values have been found" << std::endl;
  }

  Tree->delete_data();
  delete Tree;
}

//UNCOMMENT MAIN TO TEST EXAMPLES IN STANDALONE FASHION
// int main(){
//   example_1_ternary();
//   return 0;
// }
