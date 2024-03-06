### What is this repository for? ###
C++ implementation of 3 structures for the dictionary problem (associative arrays with multiple values per key).
The structures:
- Radix tree of radix = 2 (Patricia tree or Blind digital trie). (patricia.cpp and patricia.hpp)
- Ternary search tree. (ternary.cpp and ternary.hpp)
- Hash table with linear probing and table doubling. (linear_hash.cpp and linear_hash.hpp)

### How do I get set up? ###
All necessary files are included in the folder. To compile the experiments do: (It is necessary to change a single int
variable in the main of experiments.cpp in order to change the experiment executed, but it is explained in the file itself
to avoid confusing the user).

g++ -std=c++11 experiments.cpp utility.cpp linear_hash.cpp patricia.cpp ternary.cpp

This was implemented and tested under ubuntu 14.04 with g++ compiler version 4.8.4, while there is a boost dependency
in the implementation (boost::dynamic_bitset<>), I believe it is sufficient to import the header (as it is done in the
files). I thought important to mention it, in case the library is not linked in the system where the code is tested.

The code has comments to guide the inspection. But in broad strokes, the experiments.cpp file
contains the methods to run 3 experiments:

1) A random experiment: This experiment constructs, for a given i (exponent of 2) and alphabet size, a sample of random
words, inserts them into the 3 structures and queries the structures for all the words. It also queries for words
not generated in the first sample. The objective of the experiment is to study the effects of the change in alphabet size
to the time of operations and the structure size.

2) The single_book experiment: There are 16 books included, in the ./text/single_books/ folder, from which their words
are extracted and inserted into the structures. A subset of n/10 words are queried, and another n/10 random words not
contained in the text are generated, to also be queried. Each text is contracted (terminal words removed) or expanded
(text is reinserted) as necessary to comply with size requirements. This is all done automatically, and is documented in
the code.

3) The similarity experiment: Compares the texts included in the folder using the similarity metric.
--------------------------------------------------------------------------------------------------------------
There's also an "alternate" folder, which contains a patricia tree implementation that uses binary bit operations
over strings, instead of transforming the values to dynamic_bitsets. This implementation does not require boost, and 
can be used instead of the current patricia.cpp and patricia.hpp files. There is, however, a heavy performance penalty
for using this implementation, and it was only implemented to test whether the usage of bitsets was preferable. Turns out
that not only does the bitset implementation run faster, but it also creates a smaller structure in size.

There's also, in this folder, a deprecated.cpp file, which contains some ideas which were explored but were not fully
completed due to time constrains, such as a random word generator which samples from a letter distribution that mimics the
english letter frequency, an abstract class for the structures (which was deemed unnecesary given the small scope of the project
and in favor of templating). There's also an experiment which converts the values to bitsets outside of the patricia tree functions, to explore how much of an overhead this conversion produces: long story short, it accounts for ~35% of the insertion
time.
--------------------------------------------------------------------------------------------------------------
