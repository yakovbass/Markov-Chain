#include "markov_chain.h"
#include <string.h>
/**
 * Get random number between 0 and max_number [0, max_number).
 * @param max_number
 * @return Random number
 */
#define TRAVELLER 0
#define FLAG 1
#define WHICH_WORD 0
#define LEN_OF_TWEET 1

int get_random_number(int max_number)
{
    return rand() % max_number;
}
Node* get_node_from_database(MarkovChain *markov_chain, void *data_ptr){
    Node * traveller = markov_chain->database->first;
    while (traveller){
        if (markov_chain->comp_func(traveller->data->data,data_ptr) == 0){
            return traveller;
        }
        traveller = traveller->next;
    }
    return NULL;
}


Node* add_to_database(MarkovChain *markov_chain, void *data_ptr){
    MarkovNode* new_markov_node
    = (MarkovNode*) malloc(sizeof (MarkovNode));
    if (new_markov_node == NULL){
        fprintf(stdout,ALLOCATION_ERROR_MASSAGE);
        return NULL;
    }
//    new_markov_node->data = my_strdup(markov_chain,data_ptr);
    new_markov_node->data
    = markov_chain->copy_func(data_ptr);
    if (new_markov_node->data == NULL){
        return NULL;
    }
    new_markov_node->frequency_list = NULL;
    new_markov_node->following_count = 0;
    Node *new_node =
            (Node *)malloc(sizeof(Node));
    if (new_node == NULL){
        fprintf(stdout,ALLOCATION_ERROR_MASSAGE);
        return NULL;
    }
    new_node->data = new_markov_node;
    new_node->next = NULL;
    if (markov_chain->database->first == NULL) {
        // Adding the first node to the list
        markov_chain->database->first = new_node;
        markov_chain->database->last = new_node;
    } else {
        // Adding a new node at the end of the list
        markov_chain->database->last->next = new_node;
        markov_chain->database->last = new_node;
    }
    markov_chain->database->size++;
    return new_node;
}

int new_frequency_list(MarkovNode *first_node
                       , MarkovNode *second_node){
    first_node->frequency_list = (MarkovNodeFrequency *)
            malloc(sizeof(MarkovNodeFrequency));
    if (first_node->frequency_list == NULL){
        fprintf(stdout,ALLOCATION_ERROR_MASSAGE);
        return EXIT_FAILURE;
    }
    first_node->frequency_list[0].markov_node = second_node;
    first_node->frequency_list[0].frequency = 1;
    first_node->following_count = 1;
    first_node->frequency_list->num_of_nodes = 1;
    first_node->all_following = 1;
    return EXIT_SUCCESS;
}

int add_num_of_frequency(MarkovNode *first_node
                         , MarkovNode *second_node
                         , MarkovChain *markov_chain) {
    for (int i = 0; i < first_node->following_count; i++) {
        if ((markov_chain->comp_func(
                first_node->frequency_list[i].markov_node->data
                ,second_node->data)) == 0) {
            first_node->frequency_list[i].frequency++;
            first_node->all_following++;
            return EXIT_SUCCESS;
        }
    }
    return EXIT_FAILURE;
}

int add_node_to_frequency_list(MarkovNode *first_node
        , MarkovNode *second_node, MarkovChain *markov_chain){
    if (first_node->frequency_list == NULL) {
        int make_new_frequency_list
                = new_frequency_list(first_node,second_node);
        if (make_new_frequency_list == EXIT_FAILURE){
            return EXIT_FAILURE;
        }
    }
    else{
        int add_num_to_frequency_list
                = add_num_of_frequency(first_node,second_node, markov_chain);
        if (add_num_to_frequency_list == EXIT_SUCCESS){
            return EXIT_SUCCESS;
        }
        first_node->frequency_list->num_of_nodes++;
        MarkovNodeFrequency *new_list =
                (MarkovNodeFrequency *)realloc
                        (first_node->frequency_list,
                         (first_node->following_count + 1) *
                         sizeof(MarkovNodeFrequency));
        if (new_list == NULL) {
            fprintf(stdout,ALLOCATION_ERROR_MASSAGE);
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

MarkovNode* get_first_random_node(MarkovChain *markov_chain){
    Node * node_to_return = markov_chain->database->first;
    while (FLAG){
        int num = get_random_number(markov_chain->database->size);
        int traveller = TRAVELLER;
        while (traveller != num){
            node_to_return = node_to_return->next;
            traveller++;
        }
        void * one_word = node_to_return->data->data;
        //unsigned  long len_word = strlen(one_word);
        if (!(markov_chain->is_last(one_word))){
            break;
        }
        node_to_return = markov_chain->database->first;
    }
    return node_to_return->data;
}

MarkovNodeFrequency * which_node(MarkovNode *first_node, int random_num) {
    int which_word = WHICH_WORD;
    for (int i = 0; i < first_node->following_count; ++i) {
        MarkovNodeFrequency *freq = &(first_node->frequency_list[i]);
        which_word = which_word + freq->frequency;
        if (which_word > random_num) {
            return freq;
        }
    }
    MarkovNodeFrequency *freq = &(first_node->frequency_list[0]);
    return freq;
}

MarkovNode* get_next_random_node(MarkovNode *cur_markov_node){
    int random_num = get_random_number(cur_markov_node->all_following);
    MarkovNodeFrequency * freq_node = which_node(cur_markov_node,random_num);
    return freq_node->markov_node;
}

void generate_random_sequence(MarkovChain *markov_chain, MarkovNode *
first_node, int max_length){
    int len_of_tweet = LEN_OF_TWEET;
    while ((!markov_chain->is_last(first_node->data))
    && len_of_tweet < max_length){
        first_node = get_next_random_node(first_node);
        markov_chain->print_func(first_node->data);
        len_of_tweet++;
    }
}

void free_markov_chain(MarkovChain ** ptr_chain){
    if (ptr_chain == NULL || *ptr_chain == NULL) {
        return;
    }
    MarkovChain *chain = *ptr_chain;
    if (chain->database == NULL){
        free(chain);
        chain = NULL;
        return;
    }
    Node *traveller = chain->database->first;
    while (traveller){
        MarkovNode * our_node = traveller->data;
        if (our_node){
            if (our_node->data){
                chain->free_data(our_node->data);
                our_node->data = NULL;
            }
            if (our_node->frequency_list){
                free(our_node->frequency_list);
                our_node->frequency_list =NULL;
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