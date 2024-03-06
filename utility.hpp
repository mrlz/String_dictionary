#ifndef __U_H_INCLUDED__
#define __U_H_INCLUDED__

#include <random>
#include <chrono>
#include <fstream>
#include <set>
#include <iterator>
#include <sstream>
#include <bitset>
#include <algorithm>
#include <iostream>

double elapsed_time_milli(std::chrono::time_point<std::chrono::steady_clock,std::chrono::nanoseconds> start, std::chrono::time_point<std::chrono::steady_clock,std::chrono::nanoseconds> end);
bool string_compare(const std::string& a, const std::string& b);
double elapsed_time_seconds(double millis);
std::vector<std::size_t> compute_words_per_length(std::vector<std::string> words);
std::vector<std::string> split(const std::string &s, char delimiter);
std::string parse_file_txt(std::string source);
bool is_alfanum_space_or_apostrophe(char c);
bool keep_apostrophe(char c);
std::string clean_string_keep_space(std::string source);
std::vector<std::string> words_from_text_with_space(std::string filename);
std::vector<std::string> match_word_size(std::vector<std::string> words, std::size_t goal_size);
std::vector<std::string> size_i_text_from_book_sorted(std::vector<std::string> text_vector, long elements, int sorted);
std::vector<std::size_t> pick_word_lengths(double average_word_length, std::size_t words);
std::string random_word(std::size_t word_size, std::size_t alphabet_size, int starting_character);
std::vector<std::string> random_words(std::vector<std::size_t> words_per_length, int alphabet_size, int starting_character);
void print_binary_chars(std::string string);
std::vector<std::string> words_not_in_text(std::size_t total_words, int alphabet_size, int starting_character, std::vector<std::string> words, std::size_t max_word_size);
std::vector<std::string> random_words_in_text(std::size_t total_words, std::vector<std::string> words);
std::size_t total_size(std::bitset<16> combination, std::vector<std::size_t> sizes);
std::size_t difference_of_size(std::bitset<16> under_study, std::bitset<16> candidate_set, std::vector<std::size_t> sizes);
std::bitset<16> set_bits_accordingly(std::bitset<16> j, std::bitset<16> under_study);
std::size_t size_criteria(std::bitset<16> under_study, std::bitset<16> candidate, std::vector<std::size_t> sizes, int max_size, int min_size);
std::vector<std::vector<int>> without_the_same_books(std::bitset<16> under_study, std::vector<std::size_t> sizes, int threshold);
std::vector<std::vector<int>> book_sets(std::vector<std::size_t> sizes, int threshold);
std::vector<std::vector<int>> size_restricted_book_sets(std::vector<std::vector<int>> threshold_sets, int max_size, std::vector<std::size_t> sizes, int min_size);
void print_books(std::bitset<16> team_a, std::bitset<16> team_b, std::vector<std::string> book_names, std::vector<std::size_t> sizes);
void find_book_sets(std::string folder,std::string extension,std::vector<std::string> book_names, int threshold, int min_pow, int max_pow);


class Output_Manager{
public:
  Output_Manager(std::string filename);
  void set_header_type(int i);
  void set_header_variables(int alphabet_size, int i);
  void set_header_variables(std::string text_name, int i);
  void set_header_variables(std::string text1, std::string text2, int i);
  void close();
  void collect_info_and_print(std::string header, int structure);
  void print(int type);
  void prepare_vectors(int max_word_size);
  void update_values(std::string name, std::vector<double> new_data, std::vector<std::vector<double>> new_data_m, std::vector<std::size_t> words_per_length, std::vector<std::size_t> miss_words_per_size);
  void update_values(std::string name, std::vector<double> new_data);
  void compute_averages(int iterations, int type);
private:
  void print_similarity(std::string header, int structure);
  std::vector<std::string> structure_names = {"PATR", "TERN", "HASH"};
  std::ofstream output;
  std::ofstream output_by_m;
  std::vector<std::vector<double>> data;
  std::vector<std::vector<std::vector<double>>> data_by_m;
  std::string i;
  std::string alphabet_size;
  std::string text;
  std::string text2;
  std::string table_columns_1 = "insert_time(s), avg_insert(ms), search_time(s), avg_search(ms), miss_time(s), avg_miss(ms), size(bytes), extra, total_time(s), avg_total(ms)";
  std::string table_columns_2 = "search_time(ms), avg_search_time(ms), miss_time(ms), avg_miss_time(ms), m";
};

//Prints a vector of words. (Templates need more visibility than functions, hence why it is declared in hpp file)
template<typename T> void print_vector(std::vector<T> vector){
  for(int i = 0; i < vector.size(); i++){
    std::cout << vector[i] << ", ";
  }
  std::cout << std::endl;
}

#endif
