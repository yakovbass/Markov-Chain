#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "markov_chain.h"
#include "linked_list.h"
#define FILE_PATH_ERROR "Error: incorrect file path"
#define NUM_ARGS_ERROR "Usage: invalid number of arguments"
#define DELIMITERS " \n\t\r"
#define MAX_LEN_ROW 1000
#define START_CHAIN 0
#define BASE_TEN 10
#define LEN_OF_TWEETS 1
#define MAX_LEN_OF_TWEET 20
#define MIN_NUM_ARGS 4
#define MAX_NUM_ARGS 5




int is_right_path(char * path, int args){
    if (args != MIN_NUM_ARGS && args != MAX_NUM_ARGS){
        fprintf(stdout,NUM_ARGS_ERROR);
        return EXIT_FAILURE;
    }
    FILE *input_file = fopen(path, "r");
    if (input_file == NULL){
        fprintf(stdout,FILE_PATH_ERROR);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


int fill_without_limit(FILE *fp,MarkovChain * markov_chain){
    int start_chain = START_CHAIN;
    char * row = malloc(sizeof(char)*MAX_LEN_ROW);
    if (row == NULL){
        fprintf(stdout, ALLOCATION_ERROR_MASSAGE);
        return EXIT_FAILURE;
    }
    MarkovNode * save_last_one = NULL;
    while (fgets(row, MAX_LEN_ROW, fp) != NULL){
        void *token = strtok(row, DELIMITERS);
        while (token){
            Node * has_node =
                    get_node_from_database(markov_chain
                            ,(char *)token);
            if (has_node == NULL){
                has_node = add_to_database(markov_chain,(char *)token);
                if (has_node == NULL){
                    return EXIT_FAILURE;
                }
            }
            if (save_last_one != NULL){
                char * s = (char *)save_last_one->data;
                unsigned long len_s = strlen(s);
                if (s[len_s-1] != '.'){
                    int add_to_frequency_list
                            = add_node_to_frequency_list
                                    (save_last_one,
                                     has_node->data, markov_chain);
                    if (add_to_frequency_list == EXIT_FAILURE){
                        return EXIT_FAILURE;
                    }
                }
            }
            save_last_one = has_node->data;
            token = strtok(NULL,DELIMITERS);
            start_chain++;
        }
    }
    free(row);
    row = NULL;
    return EXIT_SUCCESS;
}

int fill_database(FILE *fp,long words_to_read,MarkovChain * markov_chain){
    int start_chain = START_CHAIN;
    char *row = malloc(MAX_LEN_ROW * sizeof(char));
    if (row == NULL){
        fprintf(stdout, ALLOCATION_ERROR_MASSAGE);
        return EXIT_FAILURE;
    }
    MarkovNode * save_last_one = NULL;
    while (fgets(row, MAX_LEN_ROW, fp)
           != NULL && start_chain < words_to_read){
        void *token = strtok(row, DELIMITERS);
        while (token && start_chain < words_to_read){
            Node * has_node =
                    get_node_from_database(markov_chain,
                                           (char *)token);
            if (has_node == NULL){
                has_node = add_to_database(markov_chain,
                                           token);
            }
            if (save_last_one != NULL){
                char * s = (char *)save_last_one->data;
                unsigned long len_s = strlen(s);
                if (s[len_s-1] != '.'){
                    int add_to_frequency_list
                            = add_node_to_frequency_list
                                    (save_last_one,
                                     has_node->data,
                                     markov_chain);
                    if (add_to_frequency_list == EXIT_FAILURE){
                        return EXIT_FAILURE;
                    }
                }
            }
            save_last_one = has_node->data;
            token = strtok(NULL,DELIMITERS);
            start_chain++;
        }
    }
    free(row);
    row = NULL;
    return EXIT_SUCCESS;
}


void (check_print_func)(void *data){
    //char * s = (char *)data;
    fprintf(stdout, "%s ",(char *)data);
}
int check_comp_fun(void *first_data,void * second_data){
    return strcmp((char *)first_data,(char *)second_data);
}
void * check_copy_func(void * data){
    char * s = (char *)data;
    size_t len = strlen(s) + 1;
    char *dup = malloc(len);
    if (dup != NULL) {
        strcpy(dup,s);
    }
    return dup;
}
void check_free_data(void *data){
    free((char *)data);
}
bool check_is_last(void * data){
    char * s = (char *) data;
    return (s[strlen(s)-1] == '.');
}



int main(int args, char * argv[]){
    if (is_right_path(argv[3], args) == EXIT_FAILURE){
        return EXIT_FAILURE;
    }
    LinkedList *list = (LinkedList *)malloc(sizeof(LinkedList));
    if (list == NULL){
        fprintf(stdout,ALLOCATION_ERROR_MASSAGE);
        return EXIT_FAILURE;
    }
    list->first = NULL;
    list->last = NULL;
    list->size = 0;
    MarkovChain *markov_chain = (MarkovChain *)malloc(sizeof(MarkovChain));
    if (markov_chain == NULL){
        fprintf(stdout,ALLOCATION_ERROR_MASSAGE);
        return EXIT_FAILURE;
    }
    markov_chain->database  = list;
    markov_chain->free_data = check_free_data;
    markov_chain->print_func = check_print_func;
    markov_chain->is_last = check_is_last;
    markov_chain->comp_func = check_comp_fun;
    markov_chain->copy_func = check_copy_func;
    long seed = strtol(argv[1],NULL,BASE_TEN);
    srand(seed);
    FILE *input_file = fopen(argv[3], "r");
    int make_the_chain = EXIT_FAILURE;
    if (args == MAX_NUM_ARGS){
        long long_value = strtol(argv[4], NULL,BASE_TEN);
        make_the_chain = fill_database(input_file,long_value,markov_chain);
    }else{
        make_the_chain = fill_without_limit(input_file,markov_chain);
    }
    if (make_the_chain == EXIT_FAILURE){
        return EXIT_FAILURE;
    }
    long max_tweets = strtol(argv[2], NULL, BASE_TEN);
    int num_tweets = LEN_OF_TWEETS;
    while (num_tweets <= max_tweets){
        fprintf(stdout,"Tweet %d: ",num_tweets);
        MarkovNode * first_node = get_first_random_node(markov_chain);
        markov_chain->print_func(first_node->data);
        generate_random_sequence(markov_chain,first_node,MAX_LEN_OF_TWEET);
        num_tweets++;
        fprintf(stdout,"\n");
    }
    free_markov_chain(&markov_chain);
    fclose(input_file);
    return EXIT_SUCCESS;
}


