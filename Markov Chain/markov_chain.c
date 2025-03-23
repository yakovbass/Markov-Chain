#include "markov_chain.h"
#include <string.h>

#define TRAVELLER 0
#define FLAG 1
#define WHICH_WORD 0
#define LEN_OF_TWEET 0
#define DELIMITERS " \n\t\r"
#define MAX_LEN_ROW 1000
#define START_CHAIN 0
#define LEN_OF_TWEETS 1
#define MAX_LEN_OF_TWEET 20
/**
 * Get random number between 0 and max_number [0, max_number).
 * @param max_number
 * @return Random number
 */
int get_random_number(int max_number) {
    return rand() % max_number;
}

MarkovChain * make_markov_chain() {
    MarkovChain *markov_chain = (MarkovChain *) malloc(sizeof(MarkovChain));
    if (markov_chain == NULL) {
        fprintf(stdout, ALLOCATION_ERROR_MASSAGE);
        return NULL;
    }
    LinkedList *list = (LinkedList *) malloc(sizeof(LinkedList));
    if (list == NULL) {
        fprintf(stdout, ALLOCATION_ERROR_MASSAGE);
        return NULL;
    }
    list->first = NULL;
    list->last = NULL;
    list->size = 0;
    markov_chain->database = list;
    return markov_chain;
}

Node *get_node_from_database(MarkovChain *markov_chain, char *data_ptr) {
    Node *traveller = markov_chain->database->first;
    while (traveller) {
        if (strcmp(traveller->data->data, data_ptr) == 0) {
            return traveller;
        }
        traveller = traveller->next;
    }
    return NULL;
}

char *my_strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *dup = malloc(len);
    if (dup != NULL) {
        strcpy(dup, s);
    }
    return dup;
}

Node *add_to_database(MarkovChain *markov_chain, char *data_ptr) {
    MarkovNode *new_markov_node = (MarkovNode *) malloc(sizeof(MarkovNode));
    if (new_markov_node == NULL) {
        fprintf(stdout, ALLOCATION_ERROR_MASSAGE);
        return NULL;
    }
    new_markov_node->data = my_strdup(data_ptr);
    if (new_markov_node->data == NULL) {
        free(new_markov_node);
        return NULL;
    }
    new_markov_node->frequency_list = NULL;
    new_markov_node->following_count = 0;
    int addNode = add(markov_chain->database, new_markov_node);
    if (addNode == 1) {
        return NULL;
    }
    return markov_chain->database->last;
}


int fill_database(FILE *fp, long words_to_read, MarkovChain *markov_chain) {
    int start_chain = START_CHAIN;
    char *row = malloc(MAX_LEN_ROW * sizeof(char));
    if (row == NULL) {
        fprintf(stdout, ALLOCATION_ERROR_MASSAGE);
        return EXIT_FAILURE;
    }
    MarkovNode *save_last_one = NULL;
    while (fgets(row, MAX_LEN_ROW, fp)
           != NULL && start_chain < words_to_read) {
        char *token = strtok(row, DELIMITERS);
        while (token && words_to_read != -1 && start_chain < words_to_read) {
            Node *has_node = get_node_from_database(markov_chain, token);
            if (has_node == NULL) {
                has_node = add_to_database(markov_chain, token);
            }
            int add_to_frequency_list
                    = add_node_to_frequency_list(save_last_one,
                                                 has_node->data);
            if (add_to_frequency_list == EXIT_FAILURE) {
                return EXIT_FAILURE;
            }
            save_last_one = has_node->data;
            token = strtok(NULL, DELIMITERS);
            start_chain++;
        }
    }
    free(row);
    row = NULL;
    return EXIT_SUCCESS;
}

int new_frequency_list(MarkovNode *first_node, MarkovNode *second_node) {
    first_node->frequency_list = (MarkovNodeFrequency *)
            malloc(sizeof(MarkovNodeFrequency));
    if (first_node->frequency_list == NULL) {
        fprintf(stdout, ALLOCATION_ERROR_MASSAGE);
        return EXIT_FAILURE;
    }
    first_node->frequency_list[0].markov_node = second_node;
    first_node->frequency_list[0].frequency = 1;
    first_node->following_count = 1;
    first_node->all_following = 1;
    return EXIT_SUCCESS;
}

int add_num_of_frequency(MarkovNode *first_node, MarkovNode *second_node) {
    for (int i = 0; i < first_node->following_count; i++) {
        if (strcmp(first_node->frequency_list[i].markov_node->data,
                   second_node->data) == 0) {
            first_node->frequency_list[i].frequency++;
            first_node->all_following++;
            return EXIT_SUCCESS;
        }
    }
    return EXIT_FAILURE;
}

int add_node_to_frequency_list(MarkovNode *first_node, MarkovNode *second_node) {
    if (first_node == second_node || first_node == NULL) {
        return EXIT_SUCCESS;
    }
    if (first_node->frequency_list == NULL) {
        int make_new_frequency_list
                = new_frequency_list(first_node, second_node);
        if (make_new_frequency_list == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
    } else {
        int add_num_to_frequency_list
                = add_num_of_frequency(first_node, second_node);
        if (add_num_to_frequency_list == EXIT_SUCCESS) {
            return EXIT_SUCCESS;
        }
        MarkovNodeFrequency *new_list =
                (MarkovNodeFrequency *) realloc
                        (first_node->frequency_list,
                         (first_node->following_count + 1) *
                         sizeof(MarkovNodeFrequency));
        if (new_list == NULL) {
            fprintf(stdout, ALLOCATION_ERROR_MASSAGE);
            return EXIT_FAILURE; // Memory reallocation failed
        }

// Add the new node to the frequency list
        new_list[first_node->following_count].markov_node = second_node;
        new_list[first_node->following_count].frequency = 1;
        first_node->following_count++;
        first_node->all_following++;
        first_node->frequency_list = new_list;
    }
    return EXIT_SUCCESS;
}

MarkovNode *get_first_random_node(MarkovChain *markov_chain) {
    Node *node_to_return = markov_chain->database->first;
    while (FLAG) {
        int num = get_random_number(markov_chain->database->size);
        int traveller = TRAVELLER;
        while (traveller != num) {
            node_to_return = node_to_return->next;
            traveller++;
        }
        char *one_word = node_to_return->data->data;
        unsigned long len_word = strlen(one_word);
        if (one_word[len_word - 1] != '.') {
            break;
        }
        node_to_return = markov_chain->database->first;
    }
    return node_to_return->data;
}

MarkovNodeFrequency *which_node(MarkovNode *first_node, int random_num) {
    int which_word = WHICH_WORD;
    for (int i = 0; i < first_node->following_count; i++) {
        MarkovNodeFrequency *freq = &(first_node->frequency_list[i]);
        which_word = which_word + freq->frequency;
        if (which_word > random_num) {
            return freq;
        }
    }
    MarkovNodeFrequency *freq = &(first_node->frequency_list[0]);
    return freq;
}

void make_tweets(long max_tweets, MarkovChain *markov_chain) {
    int num_tweets = LEN_OF_TWEETS;
    while (num_tweets <= max_tweets) {
        fprintf(stdout, "Tweet %d: ", num_tweets);
        MarkovNode *first_node = get_first_random_node(markov_chain);
        fprintf(stdout, "%s", first_node->data);
        generate_tweet(first_node, MAX_LEN_OF_TWEET);
        num_tweets++;
        fprintf(stdout, "\n");
    }
}


MarkovNode *get_next_random_node(MarkovNode *cur_markov_node) {
    int random_num = get_random_number(cur_markov_node->all_following);
    MarkovNodeFrequency *freq_node = which_node(cur_markov_node, random_num);
    return freq_node->markov_node;
}


void generate_tweet(MarkovNode *first_node, int max_length) {
    int len_of_tweet = LEN_OF_TWEET;
    while (first_node->data[strlen(first_node->data) - 1]
           != '.' && len_of_tweet <= max_length) {
        first_node = get_next_random_node(first_node);
        fprintf(stdout, " %s", first_node->data);
        len_of_tweet++;
    }
}


void free_database(MarkovChain **ptr_chain) {
    if (ptr_chain == NULL || *ptr_chain == NULL) {
        return;
    }
    MarkovChain *chain = *ptr_chain;
    if (chain->database == NULL) {
        free(chain);
        chain = NULL;
        return;
    }
    Node *traveller = chain->database->first;
    while (traveller) {
        MarkovNode *our_node = traveller->data;

        if (our_node) {
            if (our_node->data) {
                free(our_node->data);
                our_node->data = NULL;
            }
            if (our_node->frequency_list) {
                free(our_node->frequency_list);
                our_node->frequency_list = NULL;
            }
            free(our_node);
            our_node = NULL;
        }
        Node *next = traveller->next;
        free(traveller);
        traveller = NULL;
        traveller = next;
    }
    free(traveller);
    traveller = NULL;
    free(chain->database);
    chain->database = NULL;
    free(chain);
    chain = NULL;
    *ptr_chain = NULL;
}