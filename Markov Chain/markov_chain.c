// markov_chain.c
// --------------
// Implementation of the Markov chain logic:
// - building the database from a text file
// - maintaining follower frequency lists
// - generating random tweets

#include "markov_chain.h"
#include <string.h> // strlen(), strcpy(), strcmp(), strtok()

// Small constants used to avoid "magic numbers" in the code.
#define TRAVELLER 0
#define FLAG 1
#define WHICH_WORD 0
#define LEN_OF_TWEET 0

// Token delimiters used by strtok() when parsing the input text.
#define DELIMITERS " \n\t\r"

// Max line length read by fgets().
#define MAX_LEN_ROW 1000

// Initial counters.
#define START_CHAIN 0
#define LEN_OF_TWEETS 1

// Maximum words to print for a generated tweet (safety cap).
#define MAX_LEN_OF_TWEET 20

/**
 * Get random number between 0 and max_number [0, max_number).
 * @param max_number
 * @return Random number
 */
int get_random_number(int max_number) {
    // rand() is seeded once in main() using srand(seed).
    // This returns a value in the range [0, max_number).
    return rand() % max_number;
}

MarkovChain * make_markov_chain() {
    // Allocate the chain object.
    MarkovChain *markov_chain = (MarkovChain *) malloc(sizeof(MarkovChain));
    if (markov_chain == NULL) {
        fprintf(stdout, ALLOCATION_ERROR_MASSAGE);
        return NULL;
    }

    // Allocate and initialize the linked-list database.
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
    // Linear search over the linked list to find an existing word.
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
    // Small local replacement for strdup() (not part of ISO C).
    // Allocates a new buffer and copies the string including the '\0'.
    size_t len = strlen(s) + 1;
    char *dup = malloc(len);
    if (dup != NULL) {
        strcpy(dup, s);
    }
    return dup;
}

Node *add_to_database(MarkovChain *markov_chain, char *data_ptr) {
    // Create a new MarkovNode for a word that doesn't exist yet.
    MarkovNode *new_markov_node = (MarkovNode *) malloc(sizeof(MarkovNode));
    if (new_markov_node == NULL) {
        fprintf(stdout, ALLOCATION_ERROR_MASSAGE);
        return NULL;
    }

    // Duplicate the word so the chain owns its memory.
    new_markov_node->data = my_strdup(data_ptr);
    if (new_markov_node->data == NULL) {
        free(new_markov_node);
        return NULL;
    }

    // Initialize follower list metadata.
    new_markov_node->frequency_list = NULL;
    new_markov_node->following_count = 0;

    // Wrap the MarkovNode with a LinkedList Node and append it to the database.
    int addNode = add(markov_chain->database, new_markov_node);
    if (addNode == 1) {
        return NULL;
    }
    return markov_chain->database->last;
}

int fill_database(FILE *fp, long words_to_read, MarkovChain *markov_chain) {
    // Read the input file line by line, tokenize by whitespace, and build:
    // 1) unique MarkovNode entries in the database,
    // 2) frequency edges between consecutive tokens.
    int start_chain = START_CHAIN;
    char *row = malloc(MAX_LEN_ROW * sizeof(char));
    if (row == NULL) {
        fprintf(stdout, ALLOCATION_ERROR_MASSAGE);
        return EXIT_FAILURE;
    }

    // save_last_one tracks the previous token so we can add (prev -> current).
    MarkovNode *save_last_one = NULL;

    while (fgets(row, MAX_LEN_ROW, fp)
           != NULL && (words_to_read == -1 || start_chain < words_to_read)) {
        char *token = strtok(row, DELIMITERS);
        while (token) {
            // Ensure the current token exists in the database.
            Node *has_node = get_node_from_database(markov_chain, token);
            if (has_node == NULL) {
                has_node = add_to_database(markov_chain, token);
            }

            // Record the transition: previous word -> current word.
            int add_to_frequency_list = add_node_to_frequency_list(
                    save_last_one, has_node->data);
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
    // First time we see a follower for first_node: allocate a 1-element list.
    first_node->frequency_list = (MarkovNodeFrequency *)
            malloc(sizeof(MarkovNodeFrequency));
    if (first_node->frequency_list == NULL) {
        fprintf(stdout, ALLOCATION_ERROR_MASSAGE);
        return EXIT_FAILURE;
    }

    first_node->frequency_list[0].markov_node = second_node;
    first_node->frequency_list[0].frequency = 1;

    // Track number of distinct followers and total frequency sum.
    first_node->following_count = 1;
    first_node->all_following = 1;

    return EXIT_SUCCESS;
}

int add_num_of_frequency(MarkovNode *first_node, MarkovNode *second_node) {
    // If second_node already appears in the frequency list, bump its counter.
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
    // Add an edge first_node -> second_node.
    // If the edge already exists, only increment the frequency.
    if (first_node == second_node || first_node == NULL) {
        return EXIT_SUCCESS;
    }

    // First follower ever for this node.
    if (first_node->frequency_list == NULL) {
        int make_new_frequency_list = new_frequency_list(first_node, second_node);
        if (make_new_frequency_list == EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
    } else {
        // Try to increment existing follower frequency.
        int add_num_to_frequency_list = add_num_of_frequency(first_node, second_node);
        if (add_num_to_frequency_list == EXIT_SUCCESS) {
            return EXIT_SUCCESS;
        }

        // Otherwise, add a new follower at the end (grow the array by 1).
        MarkovNodeFrequency *new_list =
                (MarkovNodeFrequency *) realloc(
                        first_node->frequency_list,
                        (first_node->following_count + 1) * sizeof(MarkovNodeFrequency));
        if (new_list == NULL) {
            fprintf(stdout, ALLOCATION_ERROR_MASSAGE);
            return EXIT_FAILURE; // Memory reallocation failed
        }

        // Add the new follower at the end of the resized list.
        new_list[first_node->following_count].markov_node = second_node;
        new_list[first_node->following_count].frequency = 1;

        first_node->following_count++;
        first_node->all_following++;
        first_node->frequency_list = new_list;
    }

    return EXIT_SUCCESS;
}

MarkovNode *get_first_random_node(MarkovChain *markov_chain) {
    // Choose a random starting word.
    // Requirement: the first word cannot end with '.' (can't start at terminal).
    Node *node_to_return = markov_chain->database->first;

    while (FLAG) {
        int num = get_random_number(markov_chain->database->size);

        // Walk to the randomly selected index.
        int traveller = TRAVELLER;
        while (traveller != num) {
            node_to_return = node_to_return->next;
            traveller++;
        }

        // If the selected word ends with '.', try again.
        char *one_word = node_to_return->data->data;
        unsigned long len_word = strlen(one_word);
        if (one_word[len_word - 1] != '.') {
            break;
        }

        // Reset pointer and repeat selection.
        node_to_return = markov_chain->database->first;
    }

    return node_to_return->data;
}

MarkovNodeFrequency *which_node(MarkovNode *first_node, int random_num) {
    // Given random_num in [0, all_following), choose the bucket according to
    // cumulative frequencies.
    int which_word = WHICH_WORD;
    for (int i = 0; i < first_node->following_count; i++) {
        MarkovNodeFrequency *freq = &(first_node->frequency_list[i]);
        which_word = which_word + freq->frequency;
        if (which_word > random_num) {
            return freq;
        }
    }

    // Fallback (should not happen if all_following is consistent).
    MarkovNodeFrequency *freq = &(first_node->frequency_list[0]);
    return freq;
}

void make_tweets(long max_tweets, MarkovChain *markov_chain) {
    // Generate and print tweets 1..max_tweets.
    int num_tweets = LEN_OF_TWEETS;
    while (num_tweets <= max_tweets) {
        fprintf(stdout, "Tweet %d: ", num_tweets);

        // Choose a valid start node and print it.
        MarkovNode *first_node = get_first_random_node(markov_chain);
        fprintf(stdout, "%s", first_node->data);

        // Print the rest of the tweet.
        generate_tweet(first_node, MAX_LEN_OF_TWEET);

        num_tweets++;
        fprintf(stdout, "\n");
    }
}

MarkovNode *get_next_random_node(MarkovNode *cur_markov_node) {
    // Choose next node by weighted random selection over the frequency list.
    int random_num = get_random_number(cur_markov_node->all_following);
    MarkovNodeFrequency *freq_node = which_node(cur_markov_node, random_num);
    return freq_node->markov_node;
}

void generate_tweet(MarkovNode *first_node, int max_length) {
    // Continue sampling next words until we hit a terminal word (ending with
    // '.') or reach the maximum length cap.
    int len_of_tweet = LEN_OF_TWEET;
    while (first_node->data[strlen(first_node->data) - 1]
           != '.' && len_of_tweet <= max_length) {
        first_node = get_next_random_node(first_node);
        fprintf(stdout, " %s", first_node->data);
        len_of_tweet++;
    }
}

void free_database(MarkovChain **ptr_chain) {
    // Release all memory owned by the chain:
    // - each MarkovNode (word string + frequency list)
    // - each LinkedList Node wrapper
    // - the LinkedList itself and the MarkovChain object
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

    // traveller is already NULL here, but kept as-is to match your structure.
    free(traveller);
    traveller = NULL;

    free(chain->database);
    chain->database = NULL;

    free(chain);
    chain = NULL;

    *ptr_chain = NULL;
}
