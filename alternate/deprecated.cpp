// Base class
class Associative_Array {
   public:
      //pure virtual functions providing interface framework.
      virtual void insert(std::string pattern, std::size_t value, int text_index) = 0;
      virtual void delete_data() = 0;
      virtual bool search_report(std::string pattern, int text_index, int print) = 0;
      virtual std::string get_name() = 0;
      virtual std::size_t structure_size() = 0;
      virtual std::size_t occurences(std::string pattern) = 0;
      virtual double extra_measurement() = 0;
};

//Returns the number of occurences of the word s according to structure T (Patricia_Tree, Hash_Table or Ternary_Search_Tree).
//text_index is a parameter that the structures take to encode whether the occurence belongs
//to text_1 or text_2, so that the information can be maintained for 2 texts in a single structure.
//So quering with text_index == 0 returns the number of occurences in text_1 and text_index == 1 for text_2.
template<class structure> std::vector<std::size_t> count(std::string s, structure* T){
  return T->occurences(s);
}

//A simple stress test of the structures, which would late be refined into the experiments in experiments.cpp.
template <class structure> void structure_test(std::size_t number_of_words, double average_word_length, int alphabet_size, int starting_character, int print){
  std::cout << "  Generating ~" << number_of_words << " random words:" << std::flush;
  auto start_time = std::chrono::steady_clock::now();
  std::vector<std::string> words = random_words(pick_word_lengths(number_of_words, average_word_length), alphabet_size, starting_character);
  auto end_time = std::chrono::steady_clock::now();
  double milli = elapsed_time_milli(start_time, end_time);
  double elapsed_time = elapsed_time_seconds(milli);
  std::cout << " Generated " << words.size()<<". Took " << elapsed_time << " seconds. Average per word: " << milli/words.size() << "ms." << std::endl;
  std::size_t sizes = 0;

  structure* s = new structure();
  std::cout << std::endl;
  std::cout << "  Inserting generated words:" << std::flush;
  start_time = std::chrono::steady_clock::now();
  for (int i = 0; i < words.size(); i++){
    // std::cout << i << std::endl;
    if (print){
    std::cout << "  "<< words[i] << std::endl;
    }
    sizes = sizes + words[i].size();
    s->insert(words[i], i, 0);
  }
  end_time = std::chrono::steady_clock::now();
  milli = elapsed_time_milli(start_time, end_time);
  elapsed_time = elapsed_time_seconds(milli);
  std::cout << "DONE. Took " << elapsed_time << " seconds. Average per insertion: " << milli/words.size() << "ms." << std::endl;
  std::cout << "  Average word size of generated sample: " << (double)sizes/words.size() << " characters." << std::endl;
  std::cout << std::endl;
  bool are_all_keys_found = 1;
  std::cout << "  Searching each word:" << std::flush;
  start_time = std::chrono::steady_clock::now();
  for(int i = 0; i < words.size(); i++){
    if (print){
    std::cout << "    ";
    }
    are_all_keys_found = are_all_keys_found*s->search_report(words[i],0, print);
  }
  end_time = std::chrono::steady_clock::now();
  milli = elapsed_time_milli(start_time, end_time);
  elapsed_time = elapsed_time_seconds(milli);
  std::cout << "DONE. Took " << elapsed_time << " seconds. Average per search: " << milli/words.size() << "ms." << std::endl;
  if (are_all_keys_found){
    std::cout << std::endl << "  All keys were found in the structure." << std::endl;
  }
  s->delete_data();
  delete s;
}

//This test converts the key patterns into dynamic_bitsets before inserting them into the tree, to study the impact
//of conversion time.
void patricia_test_pre_convert_to_bits(std::size_t number_of_words, double average_word_length, int alphabet_size, int starting_character, int print){
  std::cout << "  Generating ~" << number_of_words << " random words:" << std::flush;
  auto start_time = std::chrono::steady_clock::now();
  std::vector<std::string> words = random_words(pick_word_lengths(number_of_words, average_word_length), alphabet_size, starting_character);
  auto end_time = std::chrono::steady_clock::now();
  double milli = elapsed_time_milli(start_time, end_time);
  double elapsed_time = elapsed_time_seconds(milli);
  std::cout << " Generated " << words.size()<<". Took " << elapsed_time << " seconds. Average per word: " << milli/words.size() << "ms." << std::endl;
  std::size_t sizes = 0;

  std::cout << "  Converting words to bit_sets: ";
  start_time = std::chrono::steady_clock::now();
  std::vector<boost::dynamic_bitset<>> bitset_words;
  for(int i = 0; i < words.size(); i++){
    bitset_words.push_back(bit_pattern(words[i]));
  }
  if (bitset_words.size() == words.size()){
    std::cout << "conversion succesful. ";
  }
  end_time = std::chrono::steady_clock::now();
  milli = elapsed_time_milli(start_time, end_time);
  elapsed_time = elapsed_time_seconds(milli);
  std::cout << "Took " << elapsed_time << " seconds. Average per word: " << milli/bitset_words.size() << "ms." << std::endl;

  Patricia_Tree* s = new Patricia_Tree();
  std::cout << std::endl;
  std::cout << "  Inserting generated words:" << std::flush;
  start_time = std::chrono::steady_clock::now();
  for (int i = 0; i < words.size(); i++){
    // std::cout << i << std::endl;
    // if (print){
    // std::cout << "  "<< words[i] << std::endl;
    // }
    // sizes = sizes + words[i].size();
    s->insert(bitset_words[i], i, 0);
  }
  end_time = std::chrono::steady_clock::now();
  milli = elapsed_time_milli(start_time, end_time);
  elapsed_time = elapsed_time_seconds(milli);
  std::cout << "DONE. Took " << elapsed_time << " seconds. Average per insertion: " << milli/words.size() << "ms." << std::endl;
  std::cout << "  Average word size of generated sample: " << (double)sizes/words.size() << " characters." << std::endl;
  std::cout << std::endl;
  bool are_all_keys_found = 1;
  std::cout << "  Searching each word:" << std::flush;
  start_time = std::chrono::steady_clock::now();
  for(int i = 0; i < words.size(); i++){
    if (print){
    std::cout << "    ";
    }
    are_all_keys_found = are_all_keys_found*s->search_report(words[i],0, print);
  }
  end_time = std::chrono::steady_clock::now();
  milli = elapsed_time_milli(start_time, end_time);
  elapsed_time = elapsed_time_seconds(milli);
  std::cout << "DONE. Took " << elapsed_time << " seconds. Average per search: " << milli/words.size() << "ms." << std::endl;
  if (are_all_keys_found){
    std::cout << std::endl << "  All keys were found in the structure." << std::endl;
  }
  s->delete_data();
  delete s;
}

//Tests the 3 structures + the pre-conversion variant of Patricia Trees.
void simple_test(std::size_t words){
  //Construct 4.000.000 random words with average length 5.2 (avg number of characters per word in english wikipedia)
  //for an alphabet of size 26, starting from the letter 'a' (char 97) to the letter 'z' (char 122). No prints of each
  //generated word.


  std::cout << std::endl << "Testing Patricia Tree{" << std::endl;
  structure_test<Patricia_Tree>(words, 5.2, 26, 97, 0);
  std::cout << "}\n" << std::endl;

  std::cout << std::endl << "Testing Patricia Tree with pre-conversion to bits{" << std::endl;
  patricia_test_pre_convert_to_bits(words, 5.2, 26, 97, 0);
  std::cout << "}\n" << std::endl;

  std::cout << "Testing Ternary Search Tree{" << std::endl;
  structure_test<Ternary_Search_Tree>(words, 5.2, 26, 97, 0);
  std::cout << "}\n" << std::endl;

  std::cout << "Testing Hash Table with Linear Probing{" << std::endl;
  structure_test<Hash_Table>(words, 5.2, 26, 97, 0);
  std::cout << "}" << std::endl;
}

//Given a vector that indicates how many words of each size should be made, constructs random words
//following a non-uniform distribution that mimics english letter frequencies. Letter are sampled
//from 'a' to 'z'.
std::vector<std::string> random_words_english_distribution(std::vector<std::size_t> words_per_length){
  std::vector<std::string> words;
  for (int word_size = 1; word_size < words_per_length.size(); word_size++){
    for(int i = 0; i < words_per_length[word_size]; i++){
      words.push_back(random_word_english_distribution(word_size));
    }
  }
  return words;
}

//Generates a random word given a size, but it samples from a non-uniform distribution that mimics the
//english letter frequency, sampling from 'a' to 'z'. For further information consult: https://en.wikipedia.org/wiki/Letter_frequency
std::string random_word_english_distribution(std::size_t word_size){
  std::uniform_real_distribution<double> distribution(0, 100);
  std::string string = "";
  for(int i = 0; i < word_size; i++){
    string.push_back(english_letter(distribution(generator)));
  }
  return string;
}

char english_letter(double random_value){
  if (random_value < 8.167){
    return 'a';
  }else if(random_value < 9.659){
    return 'b';
  }else if(random_value < 12.441){
    return 'c';
  }else if(random_value < 16.694){
    return 'd';
  }else if(random_value < 29.396){
    return 'e';
  }else if(random_value < 31.624){
    return 'f';
  }else if(random_value < 33.639){
    return 'g';
  }else if(random_value < 39.733){
    return 'h';
  }else if(random_value < 46.699){
    return 'i';
  }else if(random_value < 46.852){
    return 'j';
  }else if(random_value < 47.624){
    return 'k';
  }else if(random_value < 51.649){
    return 'l';
  }else if(random_value < 54.055){
    return 'm';
  }else if(random_value < 60.804){
    return 'n';
  }else if(random_value < 68.311){
    return 'o';
  }else if(random_value < 70.24){
    return 'p';
  }else if(random_value < 70.335){
    return 'q';
  }else if(random_value < 76.322){
    return 'r';
  }else if(random_value < 82.649){
    return 's';
  }else if(random_value < 91.705){
    return 't';
  }else if(random_value < 94.463){
    return 'u';
  }else if(random_value < 95.441){
    return 'v';
  }else if(random_value < 97.801){
    return 'w';
  }else if(random_value < 97.951){
    return 'x';
  }else if(random_value < 99.925){
    return 'y';
  }else if(random_value < 100){
    return 'z';
  }
}
