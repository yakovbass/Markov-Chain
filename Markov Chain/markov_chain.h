// markov_chain.h
// -------------
// Public API and data structures for a simple Markov Chain text generator.
//
// The program builds a database of unique words (states). For each word, it
// keeps a "frequency list" of words that can follow it, along with the number
// of times each follower appeared in the input.

#ifndef _MARKOV_CHAIN_H_
#define _MARKOV_CHAIN_H_

#include "linked_list.h"
#include <stdio.h>   // printf(), fprintf(), FILE
#include <stdlib.h>  // malloc(), free(), EXIT_SUCCESS/FAILURE
#include <stdbool.h> // bool

// Printed on allocation failures.
// (The original macro name is kept as-is to match existing code.)
#define ALLOCATION_ERROR_MASSAGE "Allocation failure: Failed to allocate" \
                                 " new memory\n"

// The Markov chain object. Owns the database and all MarkovNode allocations.
typedef struct MarkovChain {
    LinkedList *database;
} MarkovChain;

// A single state (word) in the chain.
typedef struct MarkovNode {
    char *data;                         // The word itself (owned by the chain)
    struct MarkovNodeFrequency *frequency_list; // Dynamic array of followers
    int all_following;                  // Sum of all follower frequencies
    int following_count;                // Number of different followers
} MarkovNode;

// An entry in a MarkovNode's frequency_list: follower word + occurrence count.
typedef struct MarkovNodeFrequency {
    struct MarkovNode *markov_node;
    int frequency;
} MarkovNodeFrequency;

/**
 * Check if data_ptr is in database. If so, return the Node wrapping it in
 * the markov_chain, otherwise return NULL.
 * @param markov_chain the chain to look in its database
 * @param data_ptr the data to look for
 * @return Pointer to the Node wrapping given data, NULL if state not in
 * database.
 */
Node *get_node_from_database(MarkovChain *markov_chain, char *data_ptr);

/**
 * If data_ptr in markov_chain, return its node. Otherwise, create new
 * node, add to end of markov_chain's database and return it.
 * @param markov_chain the chain to look in its database
 * @param data_ptr the data to look for
 * @return Node wrapping given data_ptr in given chain's database,
 * returns NULL in case of memory allocation failure.
 */
Node *add_to_database(MarkovChain *markov_chain, char *data_ptr);

/**
 * Add the second markov_node to the frequency list of the first markov_node.
 * If already in list, update its occurrence frequency value.
 * @param first_node
 * @param second_node
 * @return success/failure: 0 if the process was successful, 1 in
 * case of allocation error.
 */
int add_node_to_frequency_list(MarkovNode *first_node, MarkovNode *second_node);

/**
 * Free markov_chain and all of its content from memory
 * @param ptr_chain pointer to markov_chain to free
 */
void free_database(MarkovChain **ptr_chain);

/**
 * Create a new MarkovChain and return it.
 * @return the new MarkovChain, or NULL on allocation failure
 */
MarkovChain *make_markov_chain();

/**
 * Get a random number between 0 and max.
 * @param max the maximum number to generate
 * @return the random number
 */
int get_random_number(int max);

/**
 * Get the MarkovNodeFrequency from the given markov_node's frequency_list
 * that corresponds to the given random number.
 * @param cur_markov_node the current MarkovNode
 * @param random_num the random number to choose the MarkovNodeFrequency
 * @return the chosen MarkovNodeFrequency
 */
MarkovNodeFrequency *which_node(MarkovNode *cur_markov_node, int random_num);

/**
 * Fill the markov_chain's database with the words from the input_file.
 * @param input_file the file to read from
 * @param max_words the maximum number of words to read from the file (-1 = all)
 * @param markov_chain the chain to fill
 * @return 0 on success, 1 on allocation error
 */
int fill_database(FILE *input_file, long max_words, MarkovChain *markov_chain);

/**
 * Generate random tweets from the given markov_chain.
 * @param max_tweets number of tweets to generate
 * @param markov_chain the chain to generate tweets from
 */
void make_tweets(long max_tweets, MarkovChain *markov_chain);

/**
 * Get one random MarkovNode from the given markov_chain's database.
 * @param markov_chain
 * @return the random MarkovNode (not ending with '.')
 */
MarkovNode *get_first_random_node(MarkovChain *markov_chain);

/**
 * Choose randomly the next MarkovNode, depending on occurrence frequency.
 * @param cur_markov_node current MarkovNode
 * @return the next random MarkovNode
 */
MarkovNode *get_next_random_node(MarkovNode *cur_markov_node);

/**
 * Receive first_node, generate and print a random sentence out of it.
 * The sentence must have at least 2 words in it.
 * @param first_node markov_node to start with
 * @param max_length maximum number of words to generate
 */
void generate_tweet(MarkovNode *first_node, int max_length);

#endif /* _MARKOV_CHAIN_H_ */
