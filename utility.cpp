#include "utility.hpp"

std::random_device rand_dev_utility; //these will produce different sequences everytime
std::mt19937 generator_utility(rand_dev_utility());

//Takes two chrono time points and computes the elapsed time between the two in milliseconds.
double elapsed_time_milli(std::chrono::time_point<std::chrono::steady_clock,std::chrono::nanoseconds> start, std::chrono::time_point<std::chrono::steady_clock,std::chrono::nanoseconds> end){
  std::chrono::duration<double, std::milli> fp_ms = end-start;
  return fp_ms.count();
}

//Given a time in milliseconds, gives the time in seconds.
double elapsed_time_seconds(double millis){
  return millis/1000.0;
}

//Custom comparison function for strings. Sorts them by size, and if size is equal
//lexicographically.
bool string_compare(const std::string& a, const std::string& b){
  return a.size() < b.size() || a.size() == b.size() && a < b;
}

//For a vector of words counts how many words there are for a given length.
std::vector<std::size_t> compute_words_per_length(std::vector<std::string> words){
  std::vector<std::size_t> words_per_length(words[words.size()-1].size() + 1, 0);
  for(int i = 0; i < words.size(); i++){
    words_per_length[words[i].size()]++;
  }
  return words_per_length;
}

//Splits the string into a vector of strings cutting at delimiter. We will
//use ' ' (space character) as the delimiter.
std::vector<std::string> split(const std::string &s, char delimiter){
    std::vector<std::string> words;
    std::stringstream stream(s);
    std::string current_word;
    while(std::getline(stream, current_word, delimiter)){
      words.push_back(current_word);
    }
    return words;
}

//Parses a text file of name source. Returns it as a single string.
std::string parse_file_txt(std::string source){
  std::ifstream source_file(source);
  std::string str;

  source_file.seekg(0,std::ios::end);
  str.reserve(source_file.tellg());
  source_file.seekg(0,std::ios::beg);

  str.assign((std::istreambuf_iterator<char>(source_file)),std::istreambuf_iterator<char>());
  return str;
}

//Checks if a character is not alphanumeric, a space or an apostrophe.
bool is_alfanum_space_or_apostrophe(char c){
  if(std::isalnum(c) || c == ' ' || c == '\''){
    return 0;
  }
  return 1;
}

//Checks if a character is a punctuation symbol different from an apostrophe.
//This function and the previous one treat the apostrophe differenly from the
//rest of the punctuation symbols due to syntactic significance. (e.g: he's -> he s \ hes)
bool keep_apostrophe(char c){
  if(::ispunct(c) && c != '\''){
    return 1;
  }
  return 0;
}

//Uses is_alfanum_space_or_apostrophe to erase all characters that are not alphanumeric, spaces or apostrophes.
std::string clean_string_keep_space(std::string source){
  std::string contents = source;
  std::transform(contents.begin(), contents.end(), contents.begin(), [] (unsigned char c) { return std::tolower(c); });
  contents.erase(std::remove_if(contents.begin(), contents.end(), is_alfanum_space_or_apostrophe ), contents.end());
  return contents;
}

//This function parses a text as a string, replaces some relevant characters as spaces
//(these characters are found in the texts separating words or numbers.
//Their outright deletion would create words that are the composition of the original pair.)
//then cleans the text of all non-alphanumeric, space or apostrophe characters and finally separates
//words using spaces as delimiters.
std::vector<std::string> words_from_text_with_space(std::string filename){
  std::string s = parse_file_txt(filename);
  std::replace( s.begin(), s.end(), '\n', ' '); // replace all 'x' to 'y'
  std::replace( s.begin(), s.end(), '	', ' '); // First char is not a regular space, and was found in some of the texts.
  std::replace_if(s.begin(), s.end(), keep_apostrophe, ' ');
  std::string text = clean_string_keep_space(s);
  std::vector<std::string> x = split(text, ' ');
  std::vector<std::string> clean_words;
  for(int i = 0; i < x.size(); i++){
    if (x[i] != " " && x[i] != ""){
      clean_words.push_back(x[i]);
    }
  }
  return clean_words;
}

//The function checks whether the vector has the correct size, if there's extra elements
//it takes them from the back and if elements are missing it reinserts the vector to itself
//as much as necessary.
std::vector<std::string> match_word_size(std::vector<std::string> words, std::size_t goal_size){
  std::vector<std::string> copy = words;
  std::size_t index = 0;

  while(goal_size < copy.size()){
    copy.pop_back();
  }

  while(goal_size > copy.size()){
    copy.push_back(words[index]);
    index = (index+1)%words.size();
  }
  return copy;
}

//Creates a copy of the text_vector input and extends it or contracts it to fit size elements, and sorts it if sorted == 1
std::vector<std::string> size_i_text_from_book_sorted(std::vector<std::string> text_vector, long elements, int sorted){
  std::vector<std::string> copy = text_vector;
  if(elements > 0){
    copy = match_word_size(copy, elements);
  }
  if(sorted){
    std::sort(copy.begin(), copy.end(), string_compare);
  }
  return copy;
}


//This function selects the word_size for each word, following a binomial distribution.
std::vector<std::size_t> pick_word_lengths(double average_word_length, std::size_t words){
  double p = 0.5;
  int n = average_word_length/p;
  std::binomial_distribution<int> distribution(n,p); //params (n,p) trials,probability of success
  std::vector<std::size_t> vector(n+1,0);
  for (std::size_t i = 0; i < words; i++){
    int random_value = distribution(generator_utility);
    while(random_value == 0){
      random_value = distribution(generator_utility);
    }
    vector[random_value]++;
  }
  return vector;
}

//Generates a random word, given a size, sampling uniformly from the characters from starting_character to
//starting_character-1 + alphabet size. In ASCII, the character 'a' is represented by 97, so starting_character =
//97 and alphabet_size = 26 samples from 'a' to 'z'. For further information about ASCII consult: https://es.wikipedia.org/wiki/ASCII
std::string random_word(std::size_t word_size, std::size_t alphabet_size, int starting_character){
  std::uniform_int_distribution<int> distribution(starting_character, starting_character-1+alphabet_size);
  std::string string = "";
  for(int i = 0; i < word_size; i++){
    string.push_back((char)distribution(generator_utility));
  }
  return string;
}


//Given a vector that indicates how many words of each size should be made, it constructs random words
//for an alphabet with size alphabet_size, starting from character starting_character.
std::vector<std::string> random_words(std::vector<std::size_t> words_per_length, int alphabet_size, int starting_character){
  std::vector<std::string> words;
  for (int word_size = 1; word_size < words_per_length.size(); word_size++){
    for(int i = 0; i < words_per_length[word_size]; i++){
      words.push_back(random_word(word_size, alphabet_size, starting_character));
    }
  }
  return words;
}

//Prints the binary representation of a character, starting from the most significant digit.
void print_binary_chars(std::string string){
  for(int i = 0; i < string.size(); i++){
    std::string binary_representation = std::bitset<8>((int)string[i]).to_string();
    for(int j = binary_representation.size()-1; j >= 0; j--){
      std::cout << binary_representation[j];
    }
    std::cout << " ";
  }
  std::cout << "= " << string << std::endl;
}

//Generates random words, uniformly distributed in size, with the objective of
//properly representing each size. There will be, however, instances were not a single
//word of a given size can be drawn (e.g: for a binary alphabet it is almost certain that both characters will
//be in the text, so no words of size 1 can be selected).
std::vector<std::string> words_not_in_text(std::size_t total_words, int alphabet_size, int starting_character, std::vector<std::string> words, std::size_t max_word_size){
  std::vector<std::string> not_in_text;
  std::set<std::string> set_of_words(words.begin(), words.end());
  std::set<std::string>::iterator set_iter;
  std::string word;
  std::size_t size = 0;
  std::uniform_int_distribution<int> distribution(1, max_word_size);
  std::size_t i = 0;
  while(i < total_words/10){
    size = distribution(generator_utility);
    word = random_word(size, alphabet_size, starting_character);
    set_iter = set_of_words.find(word);
    if(set_iter == set_of_words.end()){
      not_in_text.push_back(word);
      i++;
    }
  }
  std::sort(not_in_text.begin(), not_in_text.end(), string_compare);
  return not_in_text;
}

//Given a vector of words, randomly selects a uniform sample of the words.
std::vector<std::string> random_words_in_text(std::size_t total_words, std::vector<std::string> words){
  std::vector<std::string> in_text;
  std::uniform_int_distribution<int> distribution(0, words.size()-1);
  std::string word;
  for(int i = 0; i < total_words/10; i++){
    word = words[distribution(generator_utility)];
    in_text.push_back(word);
  }
  std::sort(in_text.begin(), in_text.end(), string_compare);
  return in_text;
}

//Creates and output manager from a filename. It creates a file to store
//the construction and query times, and another file that separates by pattern length m.
Output_Manager::Output_Manager(std::string filename){
  this->output.open(filename+".csv");
  this->output_by_m.open(filename+"_by_m.csv");
}

//Prints the coresponding header to the files.
//0 for random, 1 for single book and 2 for similarity testing.
void Output_Manager::set_header_type(int i){
  if (i == 0){
    this->output << "Alg, i, |sigma|, " + this->table_columns_1 << std::endl;
    this->output_by_m << "Alg, i, |sigma|, " + this->table_columns_2 << std::endl;
  }else if(i == 1){
    this->output << "Alg, text, i, " + this->table_columns_1 << std::endl;
    this->output_by_m << "Alg, text, i, " + this->table_columns_2 << std::endl;
  }else if(i == 2){
    this->output << "Alg, text1, text2, i, insert_time(s), insert_avg(ms), search_time(s), search_avg(ms), size(bytes), extra, total_time, similarity" << std::endl;
  }
}

//Sets the header information for random experiments.
void Output_Manager::set_header_variables(int alphabet_size, int i){
  this->i = std::to_string(i);
  this->alphabet_size = std::to_string(alphabet_size);
}

//Sets the header information for single_book experiments.
void Output_Manager::set_header_variables(std::string text_name, int i){
  this->text = text_name;
  this->i = std::to_string(i);
}

void Output_Manager::set_header_variables(std::string text1, std::string text2, int i){
  this->set_header_variables(text1, i);
  this->text2 = text2;
}

//Closes the files.
void Output_Manager::close(){
  this->output.close();
  this->output_by_m.close();
}

//Since the information printed only differs at the level of the header
//we can use a common method for all 3 types of prints.
void Output_Manager::collect_info_and_print(std::string header, int structure){
  std::string text = header;
  for(int i = 0; i < 8; i++){
    text = text + std::to_string(this->data[i][structure]) + ", ";
  }
  text = text + std::to_string(this->data[0][structure] + this->data[2][structure] + this->data[4][structure]) + ", "; //total time = insert_time + search_time + miss_time
  text = text + std::to_string(this->data[1][structure] + this->data[3][structure] + this->data[5][structure]); //average total time
  this->output << text << std::endl;
  for(int size = 1; size < this->data_by_m[0][structure].size(); size++){
    std::string text_by_m = header;
    for(int i = 0; i < 4; i++){
      // std::cout << "adding datum " << this->data_by_m[i][structure][size] << std::endl;
      text_by_m = text_by_m + std::to_string(this->data_by_m[i][structure][size]) + ", ";
    }
    text_by_m = text_by_m + std::to_string(size);
    this->output_by_m << text_by_m << std::endl;
  }
}

void Output_Manager::print_similarity(std::string header, int structure){
  std::string text = header;
  for(int i = 0; i < 8; i++){
    text = text + std::to_string(this->data[i][structure]) + ", ";
  }
  this->output << text << std::endl;
}

//Prints to file according to type.
//0: Used for random tests.
//1: Used for single book experiments.
//2: Used for similarity testing.
void Output_Manager::print(int type){
  std::vector<std::string> structure_names = {"PATR", "TERN", "HASH"};
  for(int structure = 0; structure < 3; structure++){
    std::string header = structure_names[structure] + ", ";
    if (type == 0){
      header = header + this->i + ", " + this->alphabet_size + ", ";
      this->collect_info_and_print(header, structure);
    }else if(type == 1){
      header = header + this->text + ", " + this->i + ", ";
      this->collect_info_and_print(header, structure);
    }else if(type == 2){
      header = header + this->text + ", " + this->text2 + ", " + this->i + ", ";
      this->print_similarity(header, structure);
    }
  }
}

//Resets the storage vectors.
void Output_Manager::prepare_vectors(int max_word_size){
  this->data = std::vector<std::vector<double>>();
  for(int i = 0; i < 8; i++){
    this->data.push_back(std::vector<double>(3, 0.0));
  }

  if(max_word_size > 0){
    this->data_by_m = std::vector<std::vector<std::vector<double>>>();
    for(int i = 0; i < 4; i++){
      this->data_by_m.push_back(std::vector<std::vector<double>>());
      for(int j = 0; j < 3; j++){
        this->data_by_m[i].push_back(std::vector<double>(max_word_size+1,0.0));
      }
    }
  }
}

//This method is given the new_data vector, which adds to the data vector of the manager
//the new_data_m vector, which contains a vector with the search times of the words present in the structure (by size),
//and another vector which contains the search miss times of the words not present in the structure (also by size).
//With this information we update the search and miss times by m, and the average search and miss times by m.
void Output_Manager::update_values(std::string name, std::vector<double> new_data, std::vector<std::vector<double>> new_data_m, std::vector<std::size_t> words_per_length, std::vector<std::size_t> miss_words_per_size){
  int index = 0; //for patricia
  if(name == "TERN"){
    index = 1;
  }else if(name == "HASH"){
    index = 2;
  }
  if(new_data.size() != 8){
    std::cout << "ERROR: ELAPSED TIME VECTOR SIZE MISMATCH, SHOULD BE 8. In new_data.";
  }
  for(int i = 0; i < new_data.size(); i++){
    this->data[i][index] = this->data[i][index] + new_data[i];
  }
  if(new_data_m[0].size() != new_data_m[1].size() || new_data_m[0].size() != this->data_by_m[0][index].size()){
    std::cout << "ERROR: SEARCH AND MISS TIME VECTORS MISMATCH, THEY MUST BE SAME SIZE. IN new_data_m." << std::endl;
  }
  for(int j = 1; j < new_data_m[0].size(); j++){
    this->data_by_m[0][index][j] = this->data_by_m[0][index][j] + new_data_m[0][j];
    if(words_per_length[j]> 0){
      this->data_by_m[1][index][j] = this->data_by_m[1][index][j] + new_data_m[0][j]/words_per_length[j];
    }
    this->data_by_m[2][index][j] = this->data_by_m[2][index][j] + new_data_m[1][j];
    if(miss_words_per_size[j]>0){
      this->data_by_m[3][index][j] = this->data_by_m[3][index][j] + new_data_m[1][j]/miss_words_per_size[j];
    }
  }
}

void Output_Manager::update_values(std::string name, std::vector<double> new_data){
  int index = 0; //for patricia
  if(name == "TERN"){
    index = 1;
  }else if(name == "HASH"){
    index = 2;
  }
  for(int i = 0; i < new_data.size(); i++){
    this->data[i][index] = this->data[i][index] + new_data[i];
  }
}

// this->data_by_m[0][index][j] = this->data_by_m[0][index][j] + new_data_m[0][j];
// if(words_per_length[j]> 0){
// this->data_by_m[1][index][j] = this->data_by_m[1][index][j] + new_data_m[0][j]/words_per_length[j];
// }
// this->data_by_m[2][index][j] = this->data_by_m[2][index][j] + new_data_m[1][j];
// if(miss_words_per_size[j]>0){
// this->data_by_m[3][index][j] = this->data_by_m[3][index][j] + new_data_m[1][j]/miss_words_per_size[j];

//Averages the stored values by the number of iterations.
void Output_Manager::compute_averages(int iterations, int type){ //type 0 for similarity, 1 for the other 2
  for(int i = 0; i < 8; i++){
    for(int index = 0; index < 3; index++){
      this->data[i][index] = this->data[i][index]/iterations;
    }
  }

  if(type){
    for(int i = 0; i < 4; i++){
      for(int index = 0; index < 3; index ++){
        for(int m = 1; m < this->data_by_m[i][index].size();m++){
          this->data_by_m[i][index][m] = this->data_by_m[i][index][m]/iterations;
        }
      }
    }
  }
}


//The following set of functions were constructed to solve the problem of finding
//subsets of the provided texts that were close in word count, for the purpose
//of being able to compare full texts rather than slices.

//We will express a subset of the texts (which are 16) as a binary sequence that encodes
//whether the text belongs to the sequence or not. In this function sizes is a vector
//that has the word count of each text, so that the total word count of the subset
//can be computed from it.
std::size_t total_size(std::bitset<16> combination, std::vector<std::size_t> sizes){
  std::size_t total = 0;
  for(int i = 0; i < 16; i++){
    if(combination[i]){
      total = total + sizes[i];
    }
  }
  return total;
}

//Since we look to produce subsets that differ very little in since we're gonna need
//a handy function to compute just that.
std::size_t difference_of_size(std::bitset<16> under_study, std::bitset<16> candidate_set, std::vector<std::size_t> sizes){
  return abs(total_size(under_study, sizes) - total_size(candidate_set, sizes));
}

//This is a small improvement from computing the whole 2^16 x 2^16 possibilities, for every
//given subset of books, we want to produce a pair subset that does not contain the same
//books.
std::bitset<16> set_bits_accordingly(std::bitset<16> j, std::bitset<16> under_study){
  std::bitset<16> ans;
  int slot = 0;
  int k = 0;
  while(slot < ans.size()){
    if (j[k] && !under_study[slot]){
      ans[slot] = 1;
      slot++;
      k++;
    }else{
      ans[slot] = 0;
      slot++;
    }
  }
  return ans;
}

//This is criteria that checks whether the word count of two subsets are within expect bounds.
std::size_t size_criteria(std::bitset<16> under_study, std::bitset<16> candidate, std::vector<std::size_t> sizes, int max_size, int min_size){
  std::size_t size1 = total_size(under_study, sizes);
  std::size_t size2 = total_size(candidate, sizes);
  std::size_t max = std::max(size1,size2);
  if (max < max_size && max > min_size){
    return 1;
  }
  return 0;
}

//For a given subset (under_study) we want to produce all the subsets that are less than threshold words away, no repetitions.
std::vector<std::vector<int>> without_the_same_books(std::bitset<16> under_study, std::vector<std::size_t> sizes, int threshold){
  std::size_t chosen_books = 0;
  for(int i = 0; i < under_study.size(); i++){
    if (under_study[i]){
      chosen_books++;
    }
  }
  std::vector<std::vector<int>> solution;
  std::set<std::string> set_of_pairs;
  std::set<std::string>::iterator set_iter;
  for(int j = 1; j < pow(2,16-chosen_books); j++){
    std::bitset<16> candidate = set_bits_accordingly(std::bitset<16>(j),under_study);
    if (difference_of_size(under_study, candidate, sizes) < threshold){
      std::string key = under_study.to_string() + candidate.to_string();
      set_iter = set_of_pairs.find(key);
      if(set_iter == set_of_pairs.end()){
        set_of_pairs.insert(key);
        std::vector<int> new_pair;
        new_pair.push_back(under_study.to_ulong());
        new_pair.push_back(candidate.to_ulong());
        solution.push_back(new_pair);
      }
    }
  }
  return solution;
}

//We generate all the possible subsets of books and probe for their pairs that are less
//than threshold words away in the word count.
std::vector<std::vector<int>> book_sets(std::vector<std::size_t> sizes, int threshold){
  std::vector<std::vector<int>> solution;
  std::bitset<16> combination;
  for(int i = 1; i < pow(2,16); i++){
    std::cout << i << std::endl;
    std::vector<std::vector<int>> sol = without_the_same_books(std::bitset<16>(i), sizes, threshold);
    solution.insert( solution.end(), sol.begin(), sol.end() );
  }
  return solution;
}

//Once we have the set computed above we can restrict the pairs to a given size bracket, and see
//what we can classify.
std::vector<std::vector<int>> size_restricted_book_sets(std::vector<std::vector<int>> threshold_sets, int max_size, std::vector<std::size_t> sizes, int min_size){
  std::vector<std::vector<int>> size_restricted_sets;
  for(int i = 0; i < threshold_sets.size(); i++){
    if (size_criteria(std::bitset<16>(threshold_sets[i][0]), std::bitset<16>(threshold_sets[i][1]) , sizes, max_size, min_size)){
      size_restricted_sets.push_back(threshold_sets[i]);
    }
  }
  return size_restricted_sets;
}

//Prints the book names of each subset and the word count of each and total.
void print_books(std::bitset<16> team_a, std::bitset<16> team_b, std::vector<std::string> book_names, std::vector<std::size_t> sizes){
  std::size_t total = 0;
  std::size_t total_2 = 0;
  std::string out1 = "";
  std::string out2 = "";
  for (int i = 0; i < team_a.size(); i++){
    if(team_a[i]){
      out1 = out1 + book_names[i] +":" + std::to_string(sizes[i]) + ", ";
      total = total + sizes[i];
    }
    if(team_b[i]){
      out2 = out2 + book_names[i] +":" + std::to_string(sizes[i]) + ", ";
      total_2 = total_2 + sizes[i];
    }
  }
  std::cout << out1 << "total: " << total << std::endl;
  std::cout << "            VS" << std::endl;
  std::cout << out2 << "total: " << total_2 << std::endl;
}

//This is the function we call to use all those above and obtain the information.
void find_book_sets(std::string folder,std::string extension,std::vector<std::string> book_names, int threshold, int min_pow, int max_pow){
  std::vector<std::size_t> sizes;
  for(int i = 0; i < book_names.size(); i++){
    std::vector<std::string> t1 = words_from_text_with_space(folder+book_names[i]+extension);
    std::cout << book_names[i] << " : " << t1.size() << " words." << std::endl;
    sizes.push_back(t1.size());
  }
  std::vector<std::vector<int>> sets = book_sets(sizes, threshold);
  for(int i = min_pow; i < max_pow; i++){
    std::vector<std::vector<int>> restricted = size_restricted_book_sets(sets, pow(2,i), sizes, pow(2,i-1));
    std::cout << "PRINTING FOR SIZE " << i << "-----------------------------------" << std::endl;
    for(int j = 0; j < restricted.size(); j++){
      print_books(std::bitset<16>(restricted[j][0]), std::bitset<16>(restricted[j][1]), book_names, sizes);
    }
    std::cout << "DONE PRINTING FOR SIZE " << i << "---------------------------------" << std::endl << std::endl << std::endl;
  }
}
