#include "ternary.hpp"
#include "patricia.hpp"
#include "linear_hash.hpp"
#include "utility.hpp"

std::random_device rand_dev; //these will produce different sequences everytime
std::mt19937 generator(rand_dev());

Output_Manager* random_manager;
Output_Manager* single_text_manager;
Output_Manager* similarity_manager;

//This template takes a structure (namely: Patricia_Tree, Ternary_Search_Tree or Hash_Table) and performs
//a random experiment that consists of inserting all the words in the vector permutation_of_words, then
//searching all the inserted words in the structure and, finally, searching for words that were not inserted
//(namely: those contained in the vector words_not_in_the_text). While permutation_of_words contains the same
//words as the vector words, the vector words_per_length has the total of words for each size, which is useful
//since the vector words is sorted from shortest to longest words, this can save us time in the querying section
//while not having to re-clasify each word into a size bracket, so it was kept to soil the tests as least as
//possible. The *_experiment methods are not further divided, in the same spirit, to avoid introducing additional
//overhead to testing, and to keep blocks tightly knit.
template <class structure> void random_experiment(std::vector<std::string> permutation_of_words, std::vector<std::string> words, std::vector<std::size_t> words_per_length, int i, int alphabet_size, int verbose, std::vector<std::string> words_not_in_the_text, std::vector<std::size_t> words_per_length_not_in_text){
  std::vector<double> data(8, 0.0); //We will store the relevant times/values in this vector

  if(verbose){
    std::cout << "        Inserting generated words:" << std::flush;
  }
  auto start_time = std::chrono::steady_clock::now();
  //Here we create the structure and perform the insertions, the total time is measured
  //and an average is computed.
  structure* s = new structure();
  for (int i = 0; i < permutation_of_words.size(); i++){
    s->insert(permutation_of_words[i], i, 0);
  }
  auto end_time = std::chrono::steady_clock::now();
  double milli_time = elapsed_time_milli(start_time, end_time);

  data[0] = elapsed_time_seconds(milli_time);

  data[1] = milli_time/words.size();
  if(verbose){
    std::cout << "DONE. Took " << data[0] << " seconds. Average per insertion: " << data[1] << "ms." << std::endl;
  }

  std::vector<double> search_times_by_m(words_per_length.size(), 0.0);
  std::vector<double> miss_times_by_m(words_per_length.size(), 0.0);
  bool are_all_keys_found = 1;
  if(verbose){
    std::cout << "        Searching each word:" << std::flush;
  }
  auto m_time_start = std::chrono::steady_clock::now();
  auto m_time_end = std::chrono::steady_clock::now();
  start_time = std::chrono::steady_clock::now();
  //All the words are queried for, sorted by length
  //so the times are measured over the lump of the words of a given size.
  std::size_t slot = 0;
  for(int size = 1; size < words_per_length.size(); size++){
    m_time_start = std::chrono::steady_clock::now();
    for(int i = 0; i < words_per_length[size]; i++){
      are_all_keys_found = are_all_keys_found*s->search_report(words[slot],0, 0);
      slot++;
    }
    m_time_end = std::chrono::steady_clock::now();
    search_times_by_m[size] = elapsed_time_milli(m_time_start, m_time_end);
  }
  end_time = std::chrono::steady_clock::now();
  milli_time = elapsed_time_milli(start_time, end_time);
  data[2] = elapsed_time_seconds(milli_time);
  data[3] = milli_time/words.size();

  if(verbose){
    std::cout << "DONE. Took " << data[2] << " seconds. Average per search: " << data[3] << "ms." << std::endl;
    std::cout << "          Searching for missing words:" << std::flush;
  }

  slot = 0;
  int are_all_keys_missing = 0;

  //All the words not contained in the original set are queried for
  //and the times are measured per word length
  start_time = std::chrono::steady_clock::now();
  for(int size = 1; size < words_per_length_not_in_text.size(); size++){
    m_time_start = std::chrono::steady_clock::now();
    for(int i = 0; i < words_per_length_not_in_text[size]; i++){
      are_all_keys_missing = are_all_keys_missing + s->search_report(words_not_in_the_text[slot],0, 0);
      slot++;
    }
    m_time_end = std::chrono::steady_clock::now();
    miss_times_by_m[size] = elapsed_time_milli(m_time_start, m_time_end);
  }
  end_time = std::chrono::steady_clock::now();
  milli_time = elapsed_time_milli(start_time, end_time);
  data[4] = elapsed_time_seconds(milli_time);
  data[5] = milli_time/words_not_in_the_text.size();
  if(verbose){
    std::cout << "DONE. Took " << data[4] << " seconds. Average per search: " << data[5] << "ms." << std::endl;
  }


  if (verbose && are_all_keys_found){
    std::cout << std::endl << "         All keys inserted were found in the structure." << std::endl;
  }else if(!are_all_keys_found){
    std::cout << "ALL THE KEYS WERE NOT FOUND!!! " << s->get_name() << std::endl;
  }

  if (verbose && (are_all_keys_missing == 0)){
    std::cout << "          All keys not inserted were not found in the structure." << std::endl;
  }

  if(!are_all_keys_found){
    std::cout << "SOME KEYS THAT SHOULD  HAVE BEEN FOUND WERE NOT FOUND!!! " << s->get_name() << std::endl;
  }

  if(are_all_keys_missing > 0){
    std::cout << "SOME KEYS THAT SHOULDN'T HAVE BEEN FOUND WERE FOUND!!! " << s->get_name() << std::endl;
  }
  data[6] = s->structure_size();
  data[7] = s->extra_measurement();
  std::vector<std::vector<double>> data_by_m;

  data_by_m.push_back(search_times_by_m);
  data_by_m.push_back(miss_times_by_m);

  //random_manager is an output manager (defined in utility.cpp/hpp) which saves the measurements, computes
  //averages and then prints them to file accordingly.
  random_manager->update_values(s->get_name(), data, data_by_m, words_per_length, words_per_length_not_in_text);
  s->delete_data();
  delete s;
}

//This method generates a random sample of words and a sample of words that are not present
//in the first sample. It loops random_experiment over the 3 structures, where the words
//are inserted and queried for.
void alphabet_size_vs_size(int alphabet_size, int i, double average_word_length, int iterations, int permutations, int verbose){
  auto start_time_whole = std::chrono::steady_clock::now();
  std::size_t total_words = pow(2,i);
  std::cout << "  |Σ| = " << alphabet_size << ", words = " << total_words << " ["<< std::endl;

  int starting_character = 97;
  if(alphabet_size > 26){
    starting_character = 33;
  }

  auto start_time = std::chrono::steady_clock::now();
  auto end_time = std::chrono::steady_clock::now();
  double milli = elapsed_time_milli(start_time, end_time);
  double elapsed_time = elapsed_time_seconds(milli);

  auto loop_time_start = std::chrono::steady_clock::now();
  auto loop_time_end = std::chrono::steady_clock::now();
  //The header variables are the first line written to file by the manager
  //in the case of these experiments we want to know the |Σ| and the power of 2
  random_manager->set_header_variables(alphabet_size, i);
  for(int loop = 0; loop < iterations; loop++){ //Note that for each iteration we pick a random sample of words
    //but inside each iteration we permutate the random sample of words, in order to properly exploit the sample produced.
    if(verbose){
      std::cout << "     Generating ~" << total_words << " random words:" << std::flush;
    }
    start_time = std::chrono::steady_clock::now();
    //We select how many words per length will be randomly construted. This selection
    //is over a binomial distribution, as to emulate a normal with expectation = average word length
    std::vector<std::size_t> word_lengths = pick_word_lengths(average_word_length, total_words);
    std::vector<std::string> words = random_words(word_lengths, alphabet_size, starting_character);
    std::vector<std::string> words_permutation = words;
    end_time = std::chrono::steady_clock::now();
    milli = elapsed_time_milli(start_time, end_time);
    elapsed_time = elapsed_time_seconds(milli);

    //Compute words per length assumes that the words are sorted, so it can take the size of the last
    //word as indication of the maximum word size of the sample. The words are also sorted
    //so we can query for all the words in a given size at the same loop, which benefits timekeeping.
    std::vector<std::string> words_not_in_the_text = words_not_in_text(total_words, alphabet_size, starting_character, words, average_word_length*2);
    std::vector<std::size_t> words_per_length_not_in_text = compute_words_per_length(words_not_in_the_text);
    if(verbose){
      std::cout << "   Generated " << words.size()<<". Took " << elapsed_time << " seconds. Average per word: " << milli/words.size() << "ms." << std::endl;
    }
      std::cout << "      Iteration:" << loop << std::endl;


    for(int permutation_loop = 0; permutation_loop < permutations; permutation_loop++){
      loop_time_start = std::chrono::steady_clock::now();
      if(verbose){
        std::cout << "\n        Permutation: " << permutation_loop << "<" << std::endl;
        std::cout << "          Generating a permutation of the words: ";
      }
      start_time = std::chrono::steady_clock::now();
      //We permutate the words_permutation vector which contains the words for this iteration
      std::shuffle(words_permutation.begin(), words_permutation.end(), generator);

      end_time = std::chrono::steady_clock::now();
      if(verbose){
        std::cout << "DONE. Took " << elapsed_time_seconds(elapsed_time_milli(start_time, end_time))<< " seconds. " << std::endl << std::endl;
        std::cout << "          Testing Patricia Tree{" << std::endl;
      }

      start_time = std::chrono::steady_clock::now();
      //We perform the random experiment over the Patricia Tree structure
      //The random experiment inserts the words, queries for all of the words and queries for n/10 words not in the sample.
      random_experiment<Patricia_Tree>(words_permutation, words, word_lengths, i, alphabet_size, verbose, words_not_in_the_text, words_per_length_not_in_text);
      end_time = std::chrono::steady_clock::now();
      if(verbose){
        std::cout << "      } Took: " << elapsed_time_seconds(elapsed_time_milli(start_time, end_time)) << std::endl;
        std::cout << "\n          Testing Ternary Search Tree{" << std::endl;
      }

      start_time = std::chrono::steady_clock::now();
      random_experiment<Ternary_Search_Tree>(words_permutation, words, word_lengths, i, alphabet_size, verbose, words_not_in_the_text, words_per_length_not_in_text);
      end_time = std::chrono::steady_clock::now();
      if(verbose){
        std::cout << "      } Took: " << elapsed_time_seconds(elapsed_time_milli(start_time, end_time)) << std::endl;
        std::cout << "\n          Testing Hash Table with Linear probing{" << std::endl;
      }

      start_time = std::chrono::steady_clock::now();
      random_experiment<Hash_Table>(words_permutation, words, word_lengths, i , alphabet_size, verbose, words_not_in_the_text, words_per_length_not_in_text);
      end_time = std::chrono::steady_clock::now();
      loop_time_end = std::chrono::steady_clock::now();
      if(verbose){
        std::cout << "\n      } Took: " << elapsed_time_seconds(elapsed_time_milli(start_time, end_time)) << std::endl;
        std::cout << "\n     > Took: " << elapsed_time_seconds(elapsed_time_milli(loop_time_start, loop_time_end)) << std::endl;
      }
    }
  }
  //With this commnand the random_manager computes the averages of the information given.
  random_manager->compute_averages(iterations*permutations, 1);
  //And finally prints the data to the files.
  random_manager->print(0);
  auto end_time_whole = std::chrono::steady_clock::now();
  std::cout << "\n  ] Took: " << elapsed_time_seconds(elapsed_time_milli(start_time_whole, end_time_whole)) << std::endl;
}

//Calls the previous method for each alphabet_size and i.
void random_experiments(double average_word_length, std::vector<int> alphabet_sizes, int starting_size, int final_size, int repetitions, int permutations, int print){
  random_manager = new Output_Manager("random_experiments");
  random_manager->set_header_type(0);
  for(int i = starting_size; i <= final_size; i++){
    std::cout << "Size: " << i << std::endl;
    for(int alphabet_size = 0; alphabet_size < alphabet_sizes.size(); alphabet_size++ ){
      random_manager->prepare_vectors(average_word_length*2);
      alphabet_size_vs_size(alphabet_sizes[alphabet_size], i, average_word_length, repetitions, permutations, print);
    }
  }
  random_manager->close();
  delete random_manager;
}

//Using the count function, it computes the similarity between 2 texts by quering over the vector
//of words called dictionary (which contains the list of words of both texts, no repetitions).
//count_t1_t2 is the total number of occurences of each word in both texts.
template <class structure> double similarity(std::vector<std::string> dictionary, structure* T, std::size_t count_t1_t2){
  double sum = 0.0;
  for(int i = 0; i < dictionary.size(); i++){
    std::vector<std::size_t> count = T->occurences(dictionary[i]); //count[0] is the number of times word appears in text 1 and count[1] is the number of times word appears in text 2.
    sum = sum + abs(count[0] - count[1]);
  }
  return 1 - sum/count_t1_t2;
}

//This experiment is similar in spirit to random_experiment, but the words are taken from the texts contained in the /text/single_books/ folder.
//max_word_length corresponds to the length of the biggest word in the text.
//t1_permutation corresponds to a permutation of the words of the text, to be inserted.
//words_per_length_in_text tells the number of words to query for every word size, in reference to words_in_the_text.
//words_in_the_text contains the words to query for which are extracted randomly from the text.
//words_per_length_not_in_text and words_not_in_text are the number of words not found in the text, for each length, and the words themselves, respectively.
template <class structure> void single_text_experiment(std::size_t max_word_length, std::vector<std::string> t1_permutation, std::vector<std::size_t> words_per_length_in_text, std::vector<std::string> words_in_the_text, std::vector<std::size_t> words_per_length_not_in_text, std::vector<std::string> words_not_in_the_text){
  std::vector<double> data;
  std::vector<std::vector<double>> data_by_m;
  std::vector<double> search_times_by_m(max_word_length+1, 0.0);
  std::vector<double> miss_times_by_m(max_word_length+1, 0.0);

  auto insert_start = std::chrono::steady_clock::now();
  structure *T = new structure();
  //Just like in random experiment, we create the structures and we insert the words, taking note of the times.
  for(int i = 0; i < t1_permutation.size(); i++){
    T->insert(t1_permutation[i], i, 0);
  }
  auto insert_end = std::chrono::steady_clock::now();
  auto insert_time = elapsed_time_milli(insert_start, insert_end);
  data.push_back(elapsed_time_seconds(insert_time));
  data.push_back(insert_time/t1_permutation.size());

  auto search_in_m_start = std::chrono::steady_clock::now();
  auto search_in_m_end = std::chrono::steady_clock::now();
  std::size_t slot = 0;
  auto search_in_start = std::chrono::steady_clock::now();
  //Here we search for the words that were randomly sampled from the text.
  for(int size = 1; size < words_per_length_in_text.size(); size++){
    search_in_m_start = std::chrono::steady_clock::now();
    for(int i = 0; i < words_per_length_in_text[size]; i++){
      T->search_report(words_in_the_text[slot],0,0);
      slot++;
    }
    search_in_m_end = std::chrono::steady_clock::now();
    search_times_by_m[size] = elapsed_time_milli(search_in_m_start, search_in_m_end);
  }
  auto search_in_end = std::chrono::steady_clock::now();
  auto search_time = elapsed_time_milli(search_in_start, search_in_end);
  data.push_back(elapsed_time_seconds(search_time));
  data.push_back(search_time/words_in_the_text.size());


  auto search_off_m_start = std::chrono::steady_clock::now();
  auto search_off_m_end = std::chrono::steady_clock::now();
  slot = 0;
  auto search_off_start = std::chrono::steady_clock::now();
  //Here we query randomly generated words which are not contained in the text.
  for(int size = 1; size < words_per_length_not_in_text.size(); size++){
    search_off_m_start = std::chrono::steady_clock::now();
    for(int i = 0; i < words_per_length_not_in_text[size]; i++){
      T->search_report(words_not_in_the_text[slot],0 ,0);
      slot++;
    }
    search_off_m_end = std::chrono::steady_clock::now();
    miss_times_by_m[size] = elapsed_time_milli(search_off_m_start, search_off_m_end);
  }
  auto search_off_end = std::chrono::steady_clock::now();
  auto search_off_time = elapsed_time_milli(search_off_start, search_off_end);
  data.push_back(elapsed_time_seconds(search_off_time));
  data.push_back(search_off_time/words_not_in_the_text.size());

  data.push_back(T->structure_size());
  data.push_back(T->extra_measurement());
  data_by_m.push_back(search_times_by_m);
  data_by_m.push_back(miss_times_by_m);
  //We feed the data to the text manager, delete the structure data and return.
  single_text_manager->update_values(T->get_name(), data, data_by_m, words_per_length_in_text, words_per_length_not_in_text);
  T->delete_data();
  delete T;
}

//Performs single_text_experiment for a given set of repetitions (permutations).
void single_text_experiments(std::vector<std::string> text, int i, int permutations){

  long elements = -1;
  if(i > 0){
    elements = pow(2,i);
  }
  //words_from_text_with_space separates the words of the text into a vector, after performing the appropriate pre-processing
  std::vector<std::string> t1 = size_i_text_from_book_sorted(text, elements, 1);
  //match_word_size function will remove words from the vector if there are more words
  //than 2^i, or will add words to the vector by repeating it to reach the size.

  std::size_t max_word_size = t1[t1.size()-1].size();
  single_text_manager->prepare_vectors(max_word_size);

  //Here we sample t1.size()/10 words randomly from the text and t1.size()/10 random words not contained in the text.
  //The result comes sorted from shortest to longest words.
  std::vector<std::string> words_not_in_the_text = words_not_in_text(t1.size(), 26, 97, t1, t1[t1.size()-1].size());
  std::vector<std::string> words_in_the_text = random_words_in_text(t1.size(),t1);
  std::cout << "    max_word_size: "<< max_word_size << " "  << t1[t1.size()-1] << std::endl;
  std::cout << "    second biggest: "<< t1[t1.size()-2].size() << " "  << t1[t1.size()-2] << std::endl;

  //We compute how many words exist per length in the querying samples.
  std::vector<std::size_t> words_per_length_not_in_text = compute_words_per_length(words_not_in_the_text);
  std::vector<std::size_t> words_per_length_in_text = compute_words_per_length(words_in_the_text);

  //We copy the text vector for permutation purposes.
  std::vector<std::string> t1_permutation = t1;

  for(int permutation = 0; permutation < permutations; permutation++){
    std::cout << "    permutation:" << permutation << std::endl;
    //We permutate the vector of words and perform the experiment for each structure.
    std::shuffle(t1_permutation.begin(), t1_permutation.end(), generator);
    single_text_experiment<Patricia_Tree>(max_word_size, t1_permutation, words_per_length_in_text, words_in_the_text, words_per_length_not_in_text, words_not_in_the_text);
    single_text_experiment<Ternary_Search_Tree>(max_word_size, t1_permutation, words_per_length_in_text, words_in_the_text, words_per_length_not_in_text, words_not_in_the_text);
    single_text_experiment<Hash_Table>(max_word_size, t1_permutation, words_per_length_in_text, words_in_the_text, words_per_length_not_in_text, words_not_in_the_text);
  }
  single_text_manager->compute_averages(permutations, 1);
  single_text_manager->print(1);
}

void perform_single_text_experiments(std::vector<std::string> book_names, std::string folder, std::string extension, int permutations){
  single_text_manager = new Output_Manager("single_text_experiments");
  single_text_manager->set_header_type(1);

  std::vector<std::vector<std::string>> word_vectors;

  std::cout << "Transforming texts to word vectors" << std::endl;
  for(int i = 0; i < book_names.size(); i++){ //Pre-compute vectors of words from the texts
    word_vectors.push_back(words_from_text_with_space(folder + book_names[i] + extension));
  }

  for(int i = 0; i < book_names.size(); i++){
    std::cout << "Computing for " << book_names[i] << "{" << std::endl;
    auto start = std::chrono::steady_clock::now();
    for(int size = 10; size <= 21; size++){ //For each book we enforce a size on it, extending and contracting the book
      //when necessary.
      std::string text = folder + book_names[i] + extension;
      if(size < 21){
        auto start_inner = std::chrono::steady_clock::now();
        std::cout << "  size:" << size << ", " + book_names[i] << "{" <<std::endl;
        single_text_manager->set_header_variables(text, size);
        single_text_experiments(word_vectors[i], size, permutations);
        auto end_inner = std::chrono::steady_clock::now();
        std::cout << "  } Took: " << elapsed_time_seconds(elapsed_time_milli(start_inner, end_inner)) << std::endl;
      }else{
        single_text_experiments(word_vectors[i], -1, 10);
      }
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "} Took: " << elapsed_time_seconds(elapsed_time_milli(start,end)) << std::endl;
  }
  single_text_manager->close();
  delete single_text_manager;
}

template <class structure> double similarity_experiment(std::vector<std::string> t1_perm, std::vector<std::string> t2_perm, std::vector<std::string> t1t2){
  std::size_t count_t1_t2 = t1_perm.size() + t2_perm.size(); //Total number of occurences of all words, including repetitions.
  std::vector<double> data;
  //Create and insert all words from each text, index 0 encodes text 1 as source, and index 1 encodes text 2 as source.
  //This is used to count the occurences per text in the search phase.
  auto start = std::chrono::steady_clock::now();
  structure *T = new structure();
  for(int i = 0; i < t1_perm.size(); i++){
    T->insert(t1_perm[i], i, 0);
  }
  for(int i = 0; i < t2_perm.size(); i++){
    T->insert(t2_perm[i], i, 1);
  }
  auto end = std::chrono::steady_clock::now();
  double elapsed_time = elapsed_time_milli(start, end);
  data.push_back(elapsed_time_seconds(elapsed_time));
  data.push_back(elapsed_time/count_t1_t2);

  //This function computes the similarity value by querying all the words in the dictionary.
  start = std::chrono::steady_clock::now();
  double text_similarity = similarity(t1t2, T, count_t1_t2);
  end = std::chrono::steady_clock::now();

  elapsed_time = elapsed_time_milli(start,end);
  data.push_back(elapsed_time_seconds(elapsed_time));
  data.push_back(elapsed_time/t1t2.size()); //We don't query for all the original words, only the non-repeated dictionary.

  data.push_back(T->structure_size());
  data.push_back(T->extra_measurement());

  data.push_back(data[0] + data[2]); //Total time
  data.push_back(text_similarity);

  similarity_manager->update_values(T->get_name(), data);
  T->delete_data();
  delete T;
  return text_similarity;
}

double similarity_experiments(std::vector<std::string> t1, std::vector<std::string> t2, int i, int permutations){
  long elements = -1;
  if(i > 0){
    elements = pow(2,i);
  }

  //It's not necessary to sort them at this step, so last argument == 0
  //This produces a copy of the original texts
  std::vector<std::string> t1_permutation = size_i_text_from_book_sorted(t1, elements, 0);
  std::vector<std::string> t2_permutation = size_i_text_from_book_sorted(t2, elements, 0);

  //We'll create the "dictionary" vector, which contains all the words in the
  //first and second text, non repeated.
  std::vector<std::string> dictionary = t1_permutation;
  dictionary.insert(dictionary.end(), t2_permutation.begin(), t2_permutation.end());
  std::sort(dictionary.begin(), dictionary.end(), string_compare);
  auto last = std::unique(dictionary.begin(), dictionary.end());
  dictionary.erase(last, dictionary.end());

  std::vector<double> similarity(3, 0.0);
  for (int permutation = 0; permutation < permutations; permutation++){
    //We permutate the input words to the structure and perform the experiment permutations number of times.
    std::shuffle(t1_permutation.begin(), t1_permutation.end(), generator);
    std::shuffle(t2_permutation.begin(), t2_permutation.end(), generator);

    std::cout << "  found similarity for size " << i << ", permutation " << permutation << ". Similarity = ";
    double pat = similarity_experiment<Patricia_Tree>(t1_permutation, t2_permutation, dictionary);
    double ter = similarity_experiment<Ternary_Search_Tree>(t1_permutation, t2_permutation, dictionary);
    double has = similarity_experiment<Hash_Table>(t1_permutation, t2_permutation, dictionary);

    similarity[0] = similarity[0] + pat;
    similarity[1] = similarity[1] + ter;
    similarity[2] = similarity[2] + has;
    std::cout << pat << " Patricia| " << ter << " Ternary| " << has << " Hash|" << std::endl;

    // similarity[0] = similarity[0] + similarity_experiment<Patricia_Tree>(t1_permutation, t2_permutation, t1t2);
    // similarity[1] = similarity[1] + similarity_experiment<Ternary_Search_Tree>(t1_permutation, t2_permutation, t1t2);
    // similarity[2] = similarity[2] + similarity_experiment<Hash_Table>(t1_permutation, t2_permutation, t1t2);
  }

  similarity_manager->compute_averages(permutations, 0); //Compute averages and print to files.
  similarity_manager->print(2);
  return similarity[0]; //for printing purposes, each structure's similarity result is saved to text by the manager
}

void perform_similarity_experiments(std::vector<std::string> book_names, std::string folder, std::string extension, int permutations){
  similarity_manager = new Output_Manager("similarity_experiments");
  similarity_manager->set_header_type(2);
  double similarity = 0.0;
  std::vector<std::vector<std::string>> word_vectors;

  std::cout << "Transforming texts to word vectors" << std::endl;
  for(int i = 0; i < book_names.size(); i++){ //Pre-compute vectors of words from the texts
    word_vectors.push_back(words_from_text_with_space(folder + book_names[i] + extension));
  }

  for(int i = 0; i < book_names.size(); i++){
    std::cout << book_names[i] << " vs {" << std::endl;
    auto start_out = std::chrono::steady_clock::now();
    for(int j = i; j < book_names.size(); j++){
      std::cout << "  "<< j << " = " <<book_names[j] << ", " <<std::endl;
      std::string text_1 = folder + book_names[i] + extension;
      std::string text_2 = folder + book_names[j] + extension;
      auto start = std::chrono::steady_clock::now();
      for(int size = 10; size <= 21; size++){
        similarity_manager->prepare_vectors(-1); //we don't account for pattern length
        similarity_manager->set_header_variables(text_1, text_2, size); // WHAT ARE THE HEADER VARIABLES FOR THIS INSTANCE?
        if(size < 21){//For each book we enforce a size on it, extending and contracting the book.
          similarity_experiments(word_vectors[i], word_vectors[j], size, permutations);
        }else{//Here we compare the original text vs the other original text.
          similarity = similarity_experiments(word_vectors[i], word_vectors[j], -1, 3);
        }
      }
      auto end = std::chrono::steady_clock::now();
      std::cout << "Took: " << elapsed_time_seconds(elapsed_time_milli(start, end)) << ". Similarity: " << similarity << " ." << std::endl;
    }
    auto end_out = std::chrono::steady_clock::now();
    std::cout << "} Took: " << elapsed_time_seconds(elapsed_time_milli(start_out,end_out)) << std::endl;
  }
  similarity_manager->close();
  delete similarity_manager;
}

int main(){
  int experiment_type = 0; // Pick 0 for random experiments, 1 for single text testing and 2 for similarity testing.

  if(experiment_type == 0){
    //These experiments construct a sample of 2^i random words, where letters are picked with a uniform distribution,
    //and are thus not linguistically significant. The word sizes are picked following a binomial distribution, so there will be
    //few extremely short and extremely long words. The main advantage of this type of experiment is the freedom to change the
    //alphabet size |Σ|.
    std::vector<int> alphabet_sizes = {2,4,6,8,10,20,26,40,60,80,94}; // All these alphabet sizes will be tested. 94 is the
    //current implementation's limit because we can only rely on ASCII codes up to that size.
    double average_word_length = 5.2; // The average word length is used to pick the word sizes following a binomial distribution
    //which emulates a normal distribution with μ = average_word_length.

    //parameters are average_word_length, alphabet_sizes, starting value of i, terminal value of i, number of iterations per sample,
    //number of permutations in each iteration, and print.
    //Total words generated correspond to 2^i
    random_experiments(average_word_length, alphabet_sizes, 10, 20, 4, 3, 0);
  }else if(experiment_type == 1){
    //This section performs the experiments pertaining the construction, insertion, and querying of words
    //sampled from texts. The folder string references the included text folder, with the book_names vector
    //containing the name of every included book.
    //For reference, books 1 to 5 correspond to A song of ice and fire series, bible is the king james version of the bible
    //and book 1 to 10 of malazan correspond to Malazan Book of the Fallen series.
    std::string folder = "./text/single_books/";
    std::string extension = ".txt";
    std::vector<std::string> book_names = {"book_1", "book_2", "book_3", "book_4", "book_5", "bible", "book_1_malazan", "book_2_malazan", "book_3_malazan", "book_4_malazan", "book_5_malazan", "book_6_malazan", "book_7_malazan", "book_8_malazan", "book_9_malazan", "book_10_malazan"};
    //The last parameter corresponds to the number of permutations that will be used for each (size, book) pair.
    perform_single_text_experiments(book_names, folder, extension, 3);
  }else if(experiment_type == 2){
    // // This was used to find the sets of books that are within 5000 words of each other
    // // to make the comparison a bit more significant.
    // // std::string folder = "./text/single_books/";
    // // std::string extension = ".txt";
    // // std::vector<std::string> book_names = {"book_1", "book_2", "book_3", "book_4", "book_5", "bible", "book_1_malazan", "book_2_malazan", "book_3_malazan", "book_4_malazan", "book_5_malazan", "book_6_malazan", "book_7_malazan", "book_8_malazan", "book_9_malazan", "book_10_malazan"};
    // // Takes a folder, extension, book names, a threshold of distance , a starting power of 2 and an ending power of 2.
    // // It compares the subsets of the books in /single_books folder to find those that have a total size greater than 2^starting_power
    // // lesser than 2^ending_power and a distance from each other not greater than the threshold of distance.
    // find_book_sets(folder, extension, book_names, 5000, 10, 20);
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    std::string folder = "./text/single_books/";
    std::string extension = ".txt";
    std::vector<std::string> book_names = {"book_1", "book_2", "book_3", "book_4", "book_5", "bible", "book_1_malazan", "book_2_malazan", "book_3_malazan", "book_4_malazan", "book_5_malazan", "book_6_malazan", "book_7_malazan", "book_8_malazan", "book_9_malazan", "book_10_malazan"};
    //the last parameter corresponds to the number of permutations that will be used for each (size, book, book) triple.
    perform_similarity_experiments(book_names, folder, extension, 3);
  }else{
    //Further experiments can be performed here.
  }
}
