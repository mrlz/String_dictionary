#include "patricia.hpp"

//Transforms a char to its binary representation.
//The bitset is very memory-efficient (1 bit per value)
//but requires the size to be set at compile time, this is
//why it is abandoned in favor of the boost::dynamic_bitset
//which is dynamic, but uses a block or 8 bytes minimum to store
//data, which is why it is not preferable for short sequences
//but can be used effectively as the dictionary of the tree.
std::bitset<8> letter_to_bitset(char letter){
  return std::bitset<8>(letter);
}

//Transforms a string word to its binary representation.
boost::dynamic_bitset<> word_to_bitset(std::string word){
  int i = 0;
  boost::dynamic_bitset<> bit_word;
  while(i < word.size()){
    std::bitset<8> bit_letter = letter_to_bitset(word[i]);
    int bit_letter_slot = 0;
    for(int j = 8*i; j < 8*(i+1); j++){
      bit_word.push_back(bit_letter[bit_letter_slot]);
      bit_letter_slot++;
    }
    i++;
  }
  return bit_word;
}

bool bit_at_position(std::string pattern, std::size_t position){
  // if (position >= pattern.size()*8){
  //   return 0;
  // }
int letter = pattern[position/8];
return (letter >> (position%8)) & 1;
}

//Fast and dumb function to transform a binary sequence into a number.
int binary_number(int a0, int a1, int a2, int a3, int a4, int a5, int a6, int a7){
  return 1*a0 + 2*a1 + 4*a2 + 8*a3 + 16*a4 + 32*a5 + 64*a6 + 128*a7;
}

//Transform a bit sequence to the character it represents.
std::string bitset_to_word(boost::dynamic_bitset<> bit_pattern, int start_pos, int offset){
  std::string word = "";
  int end = start_pos + offset;
  for(int i = start_pos; i < end; i=i+8){
    word = word + (char)binary_number(bit_pattern[i],bit_pattern[i+1],bit_pattern[i+2],bit_pattern[i+3],bit_pattern[i+4],bit_pattern[i+5],bit_pattern[i+6],bit_pattern[i+7]);
  }
  return word;
}

//Checks whether a node is a leaf, by virtue of not having children.
bool is_leaf(struct node* node){
  return (node->left_child == nullptr && node->right_child == nullptr);
}

//Patricia_Tree constructor. left_offset == -1 is checked by insertion to determine
//that the root corresponds to an empty tree.
Patricia_Tree::Patricia_Tree(void){
  this->root->left_offset = -1;
  this->root->right_offset = 0;
  this->root->left_child = nullptr;
  this->root->right_child = nullptr;
  this->root->leaf = nullptr;
}

//A leaf node contains 2 numbers that allow it to unequivocally match to a string in the dictionary: a start position
//and an offset, this saves us from having to store the string in the node. It can be handy when the dictionary cannot
//be stored in main memory, but in principle, since the information of each character is being stored regardless, it would
//be better to pay the cost of a string per node (which is just 8 bytes pointer, the same as 2 ints (8 bytes according to current compiler)).
//There is, also, the issue of branches, which inner nodes must contain to point to child nodes. This implementation stores the child pointers
//and offset value of the branches in the node itself, this allows us to reuse the branch offset values as the start position
//and offset of the text in the leaf nodes, which have no children, somehow compensating the additional cost of having
//a dictionary.
int Patricia_Tree::find_max_prefix(std::string pattern, int start_pos, int offset){ //return the first position at which bit_pattern differs from a given entry of the dictionary
  int j = start_pos;
  int smallest_size = std::min(offset, (int) pattern.size()*8);
  int pattern_slot = 0;
  while(pattern_slot < smallest_size){
    if (bit_at_position(this->dictionary, j) != bit_at_position(pattern, pattern_slot)){
      return pattern_slot;
    }
    j++;
    pattern_slot++;
  }
  return pattern_slot;
}

//Checks if the text referenced by the leaf in the dictionary corresponds to the pattern being searched, in binary form.
bool Patricia_Tree::is_leaf_the_pattern(struct node* leaf, std::string pattern){
  if (this->find_max_prefix(pattern, leaf->left_offset, leaf->right_offset) == pattern.size()*8 && leaf->right_offset == pattern.size()*8){
    return 1;
  }
  return 0;
}

//This method takes the pattern to query for in the binary and navigates the tree to find a leaf that
//contains it. Note that, since the patterns are being treated bit by bit there are only 2 possible
//branches per node (since a patricia tree is a radix tree with radix = 2), so if, at any point, we
//reach a point that is greater than the length of the pattern (end_point) or is a leaf, we return.
//end_point is a separate value, because this function is also used to search for the node where the
//first different position between a new pattern and a leaf would be found, which is what we do when
//we reinsert from a leaf.
struct node *Patricia_Tree::node_search(std::string pattern, int end_point, int *final_slot, struct node* parent, int *child_type){
  struct node *current_node = this->root;
  parent->leaf = current_node;
  int current_slot = 0;
  int reached_leaf = 0;
  struct node* query_node = nullptr;
  while( current_slot < end_point){
    parent->leaf = current_node;
    if (bit_at_position(pattern, current_slot)){
      *child_type = 1;
      query_node = current_node->right_child;
    }else{
      *child_type = 0;
      query_node = current_node->left_child;
    }

    if (query_node != nullptr){
      current_slot = current_slot + (1-*child_type)*current_node->left_offset + (*child_type)*current_node->right_offset;
      current_node = query_node;
      reached_leaf = is_leaf(current_node);
    }else{
      return current_node;
    }

    if (reached_leaf){
      break;
    }
  }
  *final_slot = current_slot;
  return current_node;
}

//Basic search function, which isn't used structurally but to perform queries.
//Searches for a candidate using the previous function, up to end_point == bit_pattern.size().
//If the candidate is a leaf, and it matches the pattern then we have a match.
struct node* Patricia_Tree::search(std::string pattern){
  // boost::dynamic_bitset<> bit_pattern = word_to_bitset(pattern);
  pattern.push_back((char)1);
  int final_slot = 0;
  int child_type = 0;
  struct node* parent = new struct node();
  struct node* candidate = this->node_search(pattern, pattern.size()*8, &final_slot, parent, &child_type);
  delete parent;

  if (is_leaf(candidate) && this->is_leaf_the_pattern(candidate, pattern)){
    return candidate;
  }
  return nullptr;
}

//Inserts a word to the bitset dictionary, in binary form.
int Patricia_Tree::insert_word_to_dictionary(std::string pattern){
  int current_size = this->dictionary.size()*8;
  // for (int i = 0; i < bit_pattern.size(); i++){
  //   this->dictionary.push_back(bit_pattern[i]);
  // }
  this->dictionary = this->dictionary + pattern;
  return current_size;
}

//Inserts a word to the dictionary, using the previous function, and makes a new leaf node that contains the
//necessary information to trace back to the new addition. The text_position vector that each node has
//allows it to store the values associated with the pattern (it is an associative array after all), and the fact
//that there are two vectors in the text_position vector means that we can keep the values of 2 separate texts
//in a single structure, which will come in handy for the similarity testing. The text_index value is used to
//specify the text from which the pattern comes from, and insert accordingly.
struct node *Patricia_Tree::insert_word_and_make_leaf(std::string pattern, std::size_t value, int text_index){
  struct node * new_node = new struct node();
  new_node->left_offset = this->insert_word_to_dictionary(pattern);
  new_node->right_offset = (int)pattern.size()*8;
  new_node->right_child = nullptr;
  new_node->left_child = nullptr;
  new_node->text_position.push_back(std::vector<std::size_t> {});
  new_node->text_position.push_back(std::vector<std::size_t> {});
  new_node->text_position[text_index].push_back(value);
  return new_node;
}

//We split a leaf because we have found its binary counterpart, so a new node
//takes the leaf spot (child_type tells us whether the leaf was a right of left son)
//and it branches out to the original leaf and the new node.
struct node* Patricia_Tree::split_leaf(struct node* parent, struct node* candidate, struct node* new_pattern_node, bool branch, bool child_type){
  struct node* new_node = new struct node();
  new_node->left_offset = 1;
  new_node->right_offset = 1;
  new_node->leaf = new_pattern_node;

  if (branch){
    new_node->right_child = new_pattern_node;
    new_node->left_child = candidate;
  }else{
    new_node->left_child = new_pattern_node;
    new_node->right_child = candidate;
  }

  if(child_type){
    parent->leaf->right_child = new_node;
  }else{
    parent->leaf->left_child = new_node;
  }
  return new_node;
}

//If the new node corresponds to the missing son of the candidate, insert it as such.
void Patricia_Tree::insert_missing_son(struct node* candidate, struct node* new_pattern_node, bool branch){
  if (branch){
    candidate->right_child = new_pattern_node;
    candidate->right_offset = 1;
  }else{
    candidate->left_child = new_pattern_node;
    candidate->left_offset = 1;
  }
}

//Sometimes a new pattern differs, from those already inserted, at a point that does not have
//a node representing it, so that node (new node) is created and the new pattern is added as a child,
//with the other child node being the node that was cut-off when the split was made.
//This is equivalent to splitting a leaf, if we consider the cut-off node as the leaf, but
//it requires us to update the offsets of the parent and of the new node to preserve the old
//path jumps. Instead of using more ifs we take advantage of the deterministic nature of
//offset arithmetics.
void Patricia_Tree::split_arc(struct node* parent, struct node* candidate, struct node* new_pattern_node, bool branch, bool child_type, int difference_of_offset){
  struct node* new_node = this->split_leaf(parent, candidate, new_pattern_node, branch, child_type);
  new_node->right_offset = (branch)*(new_node->right_offset) + (1-branch)*difference_of_offset;
  new_node->left_offset = (1-branch)*(new_node->left_offset) + (branch)*difference_of_offset;
  parent->leaf->right_offset = parent->leaf->right_offset - (child_type)*difference_of_offset;
  parent->leaf->left_offset = parent->leaf->left_offset - (1-child_type)*difference_of_offset;
}

//Other times the difference of a pattern lies beyond the scope of what is being considered, so we
//extend the arc.
void Patricia_Tree::extend_arc_and_split_leaf(struct node* parent, struct node* candidate, struct node* new_pattern_node, bool branch, bool child_type, int difference_of_offset){
  this->split_leaf(parent, candidate, new_pattern_node, branch, child_type);
  parent->leaf->right_offset = parent->leaf->right_offset - child_type*difference_of_offset; //child_type == 1
  parent->leaf->left_offset = parent->leaf->left_offset - (1-child_type)*difference_of_offset;
}

//Insert calls upon this method to reinsert from a leaf, it supplies first_different_position which is the slot
//at which the pattern stored at the leaf and the new pattern differ, at the binary level. It probes the Tree
//up to the first_different_position slot of the pattern and, according to its findings decides the strategy
//for insertion.
//When the final_slot (the last position reached through offset jumps in the node_search method) is equal to
//first_different_position it means that the leaf and the new pattern differ at the next bit to be queried.
//Since the logic of probing the tree is:
//Enter the tree-> query the first bit of the pattern -> if it is 1, branch to right son and jump right offset slots of the pattern.
//                                                    -> if it is 0, branch to left son and jump left offset slots on the pattern.
//It means that we should query first_different_position to tell these patterns appart, but we do not query on leaves.
//So we add a node in that spot, to query first_different_position, and branch accordingly.
//If the difference is 0 (final slot and first_different_position are equal) but the node is not a leaf then it must be the root with only 1 child.
//To see why: Since the tree splits leaves, extends branches and splits branches note that all these operations create nodes with 2 children, so
//the only moment we have a node with a single child is when we insert the first value to the root. This goes in accordance with it being a
//compressed trie.

//If there's a difference with the last spot jumped to: too short a jump or too long a jump, we branch accordingly.
void Patricia_Tree::reinsert_from_leaf(struct node* leaf, std::string pattern, std::size_t value, int text_index, int first_different_position){
  int final_slot = 0;
  int child_type = 0;
  struct node* parent = new struct node();
  struct node* candidate = this->node_search(pattern, first_different_position, &final_slot, parent, &child_type);
  struct node* new_pattern_node = insert_word_and_make_leaf(pattern, value, text_index);
  const bool branch = bit_at_position(pattern, first_different_position);
  // std::cout << "final " << final_slot << " fdp " << first_different_position << std::endl;
  int difference_of_offset = final_slot - first_different_position;

  if (difference_of_offset == 0){
    if (is_leaf(candidate)){
      // std::cout << "split leaf" << std::endl;
      this->split_leaf(parent, candidate, new_pattern_node, branch, child_type);
    }else{
      // std::cout << "insert missing son" << std::endl;
      this->insert_missing_son(candidate, new_pattern_node, branch);
    }
  }else if (difference_of_offset > 0){ //final_slot is > first_different_position => we need to split the arc
    // std::cout << "split arc" << std::endl;
    this->split_arc(parent, candidate, new_pattern_node, branch, child_type, difference_of_offset);
  }else{
    // std::cout << "extend arc" << std::endl;
    this->extend_arc_and_split_leaf(parent, candidate, new_pattern_node, branch, child_type, difference_of_offset);
  }
  delete parent;
}

//Inserts a pattern, in binary representation, to the tree. text_index indicates whether the values
//will be stored in the vector of the first or second text. It checks if the root corresponds to an
//empty tree and creates it accordingly if necessary. If it isn't an empty tree it queries for a leaf
//that might contain the pattern. If such a leaf is found then we just add the value to the corresponding
//vector. If the leaf does not encode the pattern then we reinsert from that leaf.
//If the candidate is not a leaf, then we must reinsert from any leaf of that node. To speed this up
//each node stores a direct reference to one of its leaves.
void Patricia_Tree::insert(std::string pattern, std::size_t value, int text_index){
  pattern.push_back((char)1);
  if (this->root->left_offset == -1){
    if(bit_at_position(pattern, 0)){
      this->root->right_child = this->insert_word_and_make_leaf(pattern, value, text_index);
      this->root->left_child = nullptr;
      this->root->leaf = this->root->right_child;
      this->root->right_offset = 1;
      this->root->left_offset = 0;
    }else{
      this->root->left_child = this->insert_word_and_make_leaf(pattern, value, text_index);
      this->root->right_child = nullptr;
      this->root->leaf = this->root->left_child;
      this->root->left_offset = 1;
      this->root->right_offset = 0;
    }
    return;
  }
  int dummy_int = 2;
  struct node* parent = new struct node();
  struct node* spot = this->node_search(pattern, (int)pattern.size()*8, &dummy_int, parent, &dummy_int);
  delete parent;
  if (is_leaf(spot)){
    int first_different_position = this->find_max_prefix(pattern, spot->left_offset, spot->right_offset);
    if(pattern.size()*8 == first_different_position && pattern.size()*8 == spot->right_offset){
      spot->text_position[text_index].push_back(value);
      return;
    }
    this->reinsert_from_leaf(spot, pattern, value, text_index, first_different_position);
    return;
  }
  int first_different_position = this->find_max_prefix(pattern, spot->leaf->left_offset, spot->leaf->right_offset);
  this->reinsert_from_leaf(spot->leaf, pattern, value, text_index, first_different_position);
  return;
}

// //This is the method used to insert a pattern in string form. It merely converts it to binary and calls
// //the previous function.
// void Patricia_Tree::insert(std::string pattern, std::size_t value, int text_index){
//   boost::dynamic_bitset<> bit_pattern = word_to_bitset(pattern);
//   for(int p = 0; p<8; p++){
//     bit_pattern.push_back(0);
//   }
//   this->insert(bit_pattern, value, text_index);
// }

//This method deletes all the nodes of the Tree.
void Patricia_Tree::delete_node(struct node* node){
  if (node != nullptr){
    this->delete_node(node->left_child);
    this->delete_node(node->right_child);
    delete node;
  }
}

//Calls the previous function, made so that all 3 structures could have a common
//name to all relevant functions for experimenting.
void Patricia_Tree::delete_data(){
  this->delete_node(this->root);
}

//Finds the depth of the tree of root node.
int Patricia_Tree::find_depth(struct node* node){
  if (node != nullptr){
    int left = this->find_depth(node->left_child);
    int right = this->find_depth(node->right_child);
    return 1 + std::max(left,right);
  }
  return 0;
}

//The following functions are used to make a printable representation of the tree
//and they were adapted from the following SO answer: https://stackoverflow.com/questions/36802354/print-binary-tree-in-a-pretty-way-using-c/36810117
//Added out of curiosity.
std::vector<std::vector<cell_display>> Patricia_Tree::get_row_display(int type) {
    // start off by traversing the tree to
    // build a vector of vectors of Node pointers
    std::vector<struct node*> traversal_stack;
    std::vector< std::vector<struct node*> > rows;
    if(!root) return std::vector<std::vector<cell_display>>();

    struct node *p = this->root;
    const int max_depth = this->find_depth(p);
    rows.resize(max_depth);
    int depth = 0;
    for(;;) {
        // Max-depth Nodes are always a leaf or null
        // This special case blocks deeper traversal
        if(depth == max_depth-1) {
            rows[depth].push_back(p);
            if(depth == 0) break;
            --depth;
            continue;
        }

        // First visit to node?  Go to left child.
        if(traversal_stack.size() == depth) {
            rows[depth].push_back(p);
            traversal_stack.push_back(p);
            if(p) p = p->left_child;
            ++depth;
            continue;
        }

        // Odd child count? Go to right child.
        if(rows[depth+1].size() % 2) {
            p = traversal_stack.back();
            if(p) p = p->right_child;
            ++depth;
            continue;
        }

        // Time to leave if we get here

        // Exit loop if this is the root
        if(depth == 0) break;

        traversal_stack.pop_back();
        p = traversal_stack.back();
        --depth;
    }

    // Use rows of Node pointers to populate rows of cell_display structs.
    // All possible slots in the tree get a cell_display struct,
    // so if there is no actual Node at a struct's location,
    // its boolean "present" field is set to false.
    // The struct also contains a string representation of
    // its Node's value, created using a std::stringstream object.
    std::vector<std::vector<cell_display>> rows_disp;
    // std::stringstream ss;
    for(const auto& row : rows) {
        rows_disp.emplace_back();
        for(struct node* pn : row) {
            if(pn) {
                // ss << pn->offset;
                std::string text;
                if (type == 0){ //print node info
                  text = "(" + std::to_string(pn->left_offset) + ", " + std::to_string(pn->right_offset) + ", " + std::to_string(is_leaf(pn)) + ")";
                }else{ //print node pattern
                  if(is_leaf(pn)){
                  // text = "(" + bitset_to_word(this->dictionary, pn->left_offset, pn->right_offset) + ")";
                  text = "(" + this->dictionary.substr(pn->left_offset/8, pn->right_offset/8) + ")";
                  }else{
                    text = "(INNER, " + std::to_string(pn->left_offset) + ", " + std::to_string(pn->right_offset) +  ")";
                  }
                }
                rows_disp.back().push_back(cell_display(text));
                // rows_disp.back().push_back(cell_display(ss.str()));
                // ss = std::stringstream();
            } else {
                rows_disp.back().push_back(cell_display());
    }   }   }
    return rows_disp;
}


// row_formatter takes the vector of rows of cell_display structs
// generated by get_row_display and formats it into a test representation
// as a vector of strings
std::vector<std::string> Patricia_Tree::row_formatter(const std::vector<std::vector<cell_display>>& rows_disp){
    using s_t = std::string::size_type;

    // First find the maximum value string length and put it in cell_width
    s_t cell_width = 0;
    for(const auto& row_disp : rows_disp) {
        for(const auto& cd : row_disp) {
            if(cd.present && cd.valstr.length() > cell_width) {
                cell_width = cd.valstr.length();
    }   }   }

    // make sure the cell_width is an odd number
    if(cell_width % 2 == 0) ++cell_width;

    // formatted_rows will hold the results
    std::vector<std::string> formatted_rows;

    // some of these counting variables are related,
    // so its should be possible to eliminate some of them.
    s_t row_count = rows_disp.size();

    // this row's element count, a power of two
    s_t row_elem_count = 1 << (row_count-1);

    // left_pad holds the number of space charactes at the beginning of the bottom row
    s_t left_pad = 0;

    // Work from the level of maximum depth, up to the root
    // ("formatted_rows" will need to be reversed when done)
    for(s_t r=0; r<row_count; ++r) {
        const auto& cd_row = rows_disp[row_count-r-1]; // r reverse-indexes the row
        // "space" will be the number of rows of slashes needed to get
        // from this row to the next.  It is also used to determine other
        // text offsets.
        s_t space = (s_t(1) << r) * (cell_width + 1) / 2 - 1;
        // "row" holds the line of text currently being assembled
        std::string row;
        // iterate over each element in this row
        for(s_t c=0; c<row_elem_count; ++c) {
            // add padding, more when this is not the leftmost element
            row += std::string(c ? left_pad*2+1 : left_pad, ' ');
            if(cd_row[c].present) {
                // This position corresponds to an existing Node
                const std::string& valstr = cd_row[c].valstr;
                // Try to pad the left and right sides of the value string
                // with the same number of spaces.  If padding requires an
                // odd number of spaces, right-sided children get the longer
                // padding on the right side, while left-sided children
                // get it on the left side.
                s_t long_padding = cell_width - valstr.length();
                s_t short_padding = long_padding / 2;
                long_padding -= short_padding;
                row += std::string(c%2 ? short_padding : long_padding, ' ');
                row += valstr;
                row += std::string(c%2 ? long_padding : short_padding, ' ');
            } else {
                // This position is empty, Nodeless...
                row += std::string(cell_width, ' ');
            }
        }
        // A row of spaced-apart value strings is ready, add it to the result vector
        formatted_rows.push_back(row);

        // The root has been added, so this loop is finsished
        if(row_elem_count == 1) break;

        // Add rows of forward- and back- slash characters, spaced apart
        // to "connect" two rows' Node value strings.
        // The "space" variable counts the number of rows needed here.
        s_t left_space  = space + 1;
        s_t right_space = space - 1;
        for(s_t sr=0; sr<space; ++sr) {
            std::string row;
            for(s_t c=0; c<row_elem_count; ++c) {
                if(c % 2 == 0) {
                    row += std::string(c ? left_space*2 + 1 : left_space, ' ');
                    row += cd_row[c].present ? '/' : ' ';
                    row += std::string(right_space + 1, ' ');
                } else {
                    row += std::string(right_space, ' ');
                    row += cd_row[c].present ? '\\' : ' ';
                }
            }
            formatted_rows.push_back(row);
            ++left_space;
            --right_space;
        }
        left_pad += space + 1;
        row_elem_count /= 2;
    }

    // Reverse the result, placing the root node at the beginning (top)
    std::reverse(formatted_rows.begin(), formatted_rows.end());

    return formatted_rows;
}

// Trims an equal number of space characters from
// the beginning of each string in the vector.
// At least one string in the vector will end up beginning
// with no space characters.
void Patricia_Tree::trim_rows_left(std::vector<std::string>& rows) {
    if(!rows.size()) return;
    auto min_space = rows.front().length();
    for(const auto& row : rows) {
        auto i = row.find_first_not_of(' ');
        if(i==std::string::npos) i = row.length();
        if(i == 0) return;
        if(i < min_space) min_space = i;
    }
    for(auto& row : rows) {
        row.erase(0, min_space);
    }
}
//Here ends the adapted code.


//This method traverses the tree to build a printable representation
//of each row and then prints them. If type == 0 it gives the offsets info
//if type == 1 it prints the patterns encoded in each leaf, or INNER NODE.
void Patricia_Tree::print_tree(int type){
    int d = this->find_depth(this->root);

    // If this tree is empty, tell someone
    if(d == 0) {
        std::cout << " <empty tree>" << std::endl;
        return;
    }

    // This tree is not empty, so get a list of node values...
    const auto rows_disp = get_row_display(type);
    // then format these into a text representation...
    auto formatted_rows = row_formatter(rows_disp);
    // then trim excess space characters from the left sides of the text...
    trim_rows_left(formatted_rows);
    // then dump the text to cout.
    for(const auto& row : formatted_rows) {
        std::cout << ' ' << row << '\n';
    }
  }

//prints the values associated with the key that the node represents.
void Patricia_Tree::print_positions(struct node* node, int text_index){
  for(int i = 0; i < node->text_position[text_index].size(); i++){
    std::cout << node->text_position[text_index][i] << ", ";
  }
}

//Searches for a pattern and reports whether it finds it or not. If print is enabled, it
//also prints the positions for a given text.
bool Patricia_Tree::search_report(std::string pattern, int text_index, int print){
  struct node* candidate = this->search(pattern);
  if (candidate != nullptr){
    if(print){
      std::cout << pattern << " found with "<< candidate->text_position[text_index].size() <<" occurences at positions: ";
      this->print_positions(candidate, text_index);
      std::cout << std::endl;
    }
    return 1;
  }
  return 0;
}

//Computes the total size used to encode occurences (values) associated with a key.
//Note that we use the capacity function of the vector class, which tells us the allocated space
//for the vector (in terms of elements), which might differ from the number of stored elements.
std::size_t Patricia_Tree::pattern_occurences_cost(std::vector<std::vector<std::size_t>> occurences){
  if(occurences.size() > 0){
    return sizeof(std::size_t)*(occurences[0].capacity() + occurences[1].capacity());
  }
  return 0;
}

//Computes the size of the subtree rooted at node.
std::size_t Patricia_Tree::node_cost(struct node* node){
  if(node != nullptr){
    return node->text_position.capacity()*sizeof(std::vector<std::size_t>) + sizeof(struct node) + this->node_cost(node->left_child) + this->node_cost(node->right_child) + this->pattern_occurences_cost(node->text_position);
  }
  return 0;
}

//Returns the total size of the tree, in bytes.
std::size_t Patricia_Tree::structure_size(){
  return sizeof(Patricia_Tree) + this->dictionary.capacity()*sizeof(char) + this->node_cost(this->root) + this->name.capacity()*sizeof(char);
}

//Returns all occurences of the pattern in the text represented by text_index.
std::size_t Patricia_Tree::occurences(std::string pattern, int text_index){
  struct node* candidate = this->search(pattern);
  return candidate->text_position[text_index].size();
}

void Patricia_Tree::print_dictionary(){
  std::cout << this->dictionary << std::endl;
}

//The following are small, plainly laid out, examples of the execution of the tree.
void example_1_patricia(){
  int print_type = 1;
  Patricia_Tree *Tree = new Patricia_Tree();
  Tree->print_tree(print_type);
  Tree->insert("hola",0,0);
  Tree->print_tree(print_type);
  Tree->insert("ola",1,0);
  Tree->print_tree(print_type);
  Tree->insert("holograma",2,0);
  Tree->print_tree(print_type);
  Tree->insert("ho",3,0);
  Tree->print_tree(print_type);
  Tree->insert("holografia",4,0);
  Tree->print_tree(print_type);

  Tree->search_report("hola", 0, 1);
  Tree->search_report("holograma", 0, 1);
  Tree->search_report("ola", 0, 1);
  Tree->search_report("ho", 0, 1);
  Tree->search_report("holografia", 0, 1);
  Tree->delete_data();
  delete Tree;
}

void example_2_patricia(){
  int print_type = 1;
  Patricia_Tree *Tree = new Patricia_Tree();
  Tree->print_tree(print_type);
  Tree->insert("ola",0,0);
  Tree->print_tree(print_type);
  Tree->insert("hola",1,0);
  Tree->print_tree(print_type);
  Tree->insert("holograma",2,0);
  Tree->print_tree(print_type);
  Tree->insert("holografia",3,0);
  Tree->print_tree(print_type);
  Tree->insert("ho",4,0);
  Tree->print_tree(print_type);

  Tree->search_report("hola", 0, 1);
  Tree->search_report("holograma", 0, 1);
  Tree->search_report("ola", 0, 1);
  Tree->search_report("ho", 0, 1);
  Tree->search_report("holografia", 0, 1);

  Tree->delete_data();
  delete Tree;
}

void example_3_patricia(){
  int print_type = 1;
  Patricia_Tree *Tree = new Patricia_Tree();
  Tree->print_tree(print_type);
  Tree->insert("SOME",10,0);
  Tree->print_tree(print_type);
  Tree->insert("ABACUS",20,0);
  Tree->print_tree(print_type);
  Tree->insert("SOMETHING",30,0);
  Tree->print_tree(print_type);
  Tree->insert("B",40,0);
  Tree->print_tree(print_type);
  Tree->insert("ABRACADABRA",50,0);
  Tree->print_tree(print_type);
  Tree->insert("THIS",60,0);
  Tree->print_tree(print_type);
  Tree->insert("SOMERSET",70,0);
  Tree->print_tree(print_type);

  Tree->search_report("SOME", 0, 1);
  Tree->search_report("ABACUS", 0, 1);
  Tree->search_report("SOMETHING", 0, 1);
  Tree->search_report("B", 0, 1);
  Tree->search_report("ABRACADABRA", 0, 1);
  Tree->search_report("THIS", 0, 1);
  Tree->search_report("SOMERSET", 0, 1);

  Tree->delete_data();
  delete Tree;
}

void example_4_patricia(){
  int print_type = 1;
  Patricia_Tree *Tree = new Patricia_Tree();
  Tree->print_tree(print_type);
  Tree->insert("ho",1233,0);
  Tree->insert("ola",0,0);
  Tree->insert("ola",10,0);
  Tree->insert("ola",100,0);
  Tree->print_tree(print_type);
  Tree->insert("hola",1,0);
  Tree->insert("hola",50,0);
  Tree->print_tree(print_type);
  Tree->insert("holografia",1000,0);
  Tree->insert("holograma",2,0);
  Tree->print_tree(print_type);
  Tree->insert("holografia",3,0);
  Tree->insert("holografia",333,0);
  Tree->print_tree(print_type);
  Tree->insert("ho",4,0);
  Tree->insert("ho",12,0);
  Tree->insert("ho",412,0);
  Tree->insert("ho",500,0);
  Tree->print_tree(print_type);

  Tree->search_report("hola", 0, 1);
  Tree->search_report("holograma", 0, 1);
  Tree->search_report("ola", 0, 1);
  Tree->search_report("ho", 0, 1);
  Tree->search_report("holografia", 0, 1);
  Tree->print_dictionary();
  Tree->delete_data();
  delete Tree;
}

//UNCOMMENT MAIN TO TEST EXAMPLES IN STANDALONE FASHION
// int main(){
// // example_3_patricia();
// return 0;
// }
