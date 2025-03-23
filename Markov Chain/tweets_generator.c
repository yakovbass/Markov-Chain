#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "markov_chain.h"
#define FILE_PATH_ERROR "Error: incorrect file path"
#define NUM_ARGS_ERROR "Usage: invalid number of arguments"
#define BASE_TEN 10
#define MIN_NUM_ARGS 4
#define MAX_NUM_ARGS 5

int is_right_path(char *path, int args) {
    if (args != MIN_NUM_ARGS && args != MAX_NUM_ARGS) {
        fprintf(stdout, NUM_ARGS_ERROR);
        return EXIT_FAILURE;
    }
    FILE *input_file = fopen(path, "r");
    if (input_file == NULL) {
        fprintf(stdout, FILE_PATH_ERROR);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}




int main(int args, char *argv[]) {
    if (is_right_path(argv[3], args) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    MarkovChain *markov_chain = make_markov_chain();
    long seed = strtol(argv[1], NULL, BASE_TEN);
    srand(seed);
    FILE *input_file = fopen(argv[3], "r");
    int make_the_chain = EXIT_FAILURE;
    if (args == MAX_NUM_ARGS) {
        long long_value = strtol(argv[4], NULL, BASE_TEN);
        make_the_chain = fill_database(input_file, long_value, markov_chain);
    } else {
        make_the_chain = fill_database(input_file, -1,markov_chain);
    }
    if (make_the_chain == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    long max_tweets = strtol(argv[2], NULL, BASE_TEN);
    make_tweets(max_tweets, markov_chain);
    free_database(&markov_chain);
    fclose(input_file);
    return EXIT_SUCCESS;
}


