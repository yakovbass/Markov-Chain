// tweets_generator.c
// ------------------
// Program entry point for generating tweets using a Markov chain.
//
// Expected CLI:
//   <seed> <num_tweets> <input_file_path> [<max_words_to_read>]
//
// If max_words_to_read is provided, only that many words are read from the
// input file. Otherwise the whole file is used.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "markov_chain.h"

#define FILE_PATH_ERROR "Error: incorrect file path"
#define NUM_ARGS_ERROR "Usage: invalid number of arguments"
#define BASE_TEN 10
#define MIN_NUM_ARGS 4
#define MAX_NUM_ARGS 5

// Validate command line argument count and that the file path is readable.
// Returns EXIT_SUCCESS on success, EXIT_FAILURE on invalid input.
int is_right_path(char *path, int args) {
    if (args != MIN_NUM_ARGS && args != MAX_NUM_ARGS) {
        fprintf(stdout, NUM_ARGS_ERROR);
        return EXIT_FAILURE;
    }

    // Try opening the file to verify it exists and is readable.
    FILE *input_file = fopen(path, "r");
    if (input_file == NULL) {
        fprintf(stdout, FILE_PATH_ERROR);
        return EXIT_FAILURE;
    }

    // Close immediately; main() will reopen it for actual reading.
    fclose(input_file);
    return EXIT_SUCCESS;
}

int main(int args, char *argv[]) {
    // argv[1] = seed, argv[2] = number of tweets, argv[3] = file path
    if (is_right_path(argv[3], args) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    // Build an empty Markov chain object.
    MarkovChain *markov_chain = make_markov_chain();
    if (markov_chain == NULL) {
        // Allocation error message already printed by make_markov_chain().
        return EXIT_FAILURE;
    }

    // Seed PRNG (used by get_random_number()).
    long seed = strtol(argv[1], NULL, BASE_TEN);
    srand(seed);

    // Open the input file for reading the corpus.
    FILE *input_file = fopen(argv[3], "r");
    if (input_file == NULL) {
        fprintf(stdout, FILE_PATH_ERROR);
        free_database(&markov_chain);
        return EXIT_FAILURE;
    }

    // Build the Markov database (either limited words or full file).
    int make_the_chain = EXIT_FAILURE;
    if (args == MAX_NUM_ARGS) {
        long tweet_length = strtol(argv[4], NULL, BASE_TEN);
        // Build the database from at most tweet_length words.
        make_the_chain = fill_database(input_file, tweet_length, markov_chain);
    } else {
        // Build the database from the entire file.
        make_the_chain = fill_database(input_file, -1, markov_chain);
    }

    if (make_the_chain == EXIT_FAILURE) {
        // Clean up before exiting on error.
        fclose(input_file);
        free_database(&markov_chain);
        return EXIT_FAILURE;
    }

    // Generate and print the requested amount of tweets.
    long max_tweets = strtol(argv[2], NULL, BASE_TEN);
    make_tweets(max_tweets, markov_chain);

    // Cleanup.
    free_database(&markov_chain);
    fclose(input_file);

    return EXIT_SUCCESS;
}
