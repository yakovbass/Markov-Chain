#include <string.h> // For strlen(), strcmp(), strcpy()
#include "markov_chain.h"

#define MAX(X, Y) (((X) < (Y)) ? (Y) : (X))

#define EMPTY -1
#define BOARD_SIZE 100
# define CURR_WALK 1
#define MAX_GENERATION_LENGTH 60
#define BASE_TEN 10
#define DICE_MAX 6
#define HEAD 0
#define TAIL 1
#define NUM_OF_TRANSITIONS 20
#define NUM_ARGS 3
#define NUM_ARGS_ERROR "Usage: invalid number of arguments"

/**
 * represents the transitions by ladders and snakes in the game
 * each tuple (x,y) represents a ladder from x to if x<y or a snake otherwise
 */
const int transitions[][2] = {{13, 4},
                              {85, 17},
                              {95, 67},
                              {97, 58},
                              {66, 89},
                              {87, 31},
                              {57, 83},
                              {91, 25},
                              {28, 50},
                              {35, 11},
                              {8,  30},
                              {41, 62},
                              {81, 43},
                              {69, 32},
                              {20, 39},
                              {33, 70},
                              {79, 99},
                              {23, 76},
                              {15, 47},
                              {61, 14}};

/**
 * struct represents a Cell in the game board
 */
typedef struct Cell {
    int number; // Cell number 1-100
    int ladder_to;  // ladder_to represents
    // the jump of the ladder in case there is one from this square
    int snake_to;  // snake_to represents the
    // jump of the snake in case there is one from this square
    //both ladder_to and snake_to should be
    // -1 if the Cell doesn't have them
} Cell;

/** Error handler **/

int is_right_num_args(int args){
    if (args != NUM_ARGS){
        fprintf(stdout,NUM_ARGS_ERROR);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}


int handle_error_snakes(char *error_msg, MarkovChain **database)
{
    printf("%s", error_msg);
    if (database != NULL)
    {
        free_markov_chain(database);
    }
    return EXIT_FAILURE;
}


int create_board(Cell *cells[BOARD_SIZE])
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        cells[i] = malloc(sizeof(Cell));
        if (cells[i] == NULL)
        {
            for (int j = 0; j < i; j++) {
                free(cells[j]);
            }
            handle_error_snakes
            (ALLOCATION_ERROR_MASSAGE,NULL);
            return EXIT_FAILURE;
        }
        *(cells[i]) = (Cell) {i + 1, EMPTY, EMPTY};
    }

    for (int i = 0; i < NUM_OF_TRANSITIONS; i++)
    {
        int from = transitions[i][0];
        int to = transitions[i][1];
        if (from < to)
        {
            cells[from - 1]->ladder_to = to;
        }
        else
        {
            cells[from - 1]->snake_to = to;
        }
    }
    return EXIT_SUCCESS;
}

/**
 * fills database
 * @param markov_chain
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int fill_database_snakes(MarkovChain *markov_chain){
    Cell* cells[BOARD_SIZE];
    if(create_board(cells) == EXIT_FAILURE){
        return EXIT_FAILURE;}
    MarkovNode *from_node = NULL, *to_node = NULL;
    size_t index_to;
    for (size_t i = 0; i < BOARD_SIZE; i++){
        add_to_database(markov_chain, cells[i]);}
    for (size_t i = 0; i < BOARD_SIZE; i++){
        from_node =
                get_node_from_database(markov_chain,cells[i])->data;
        if (cells[i]->snake_to != EMPTY || cells[i]->ladder_to != EMPTY){
            index_to = MAX(cells[i]->snake_to,cells[i]->ladder_to) - 1;
            to_node = get_node_from_database(markov_chain
                    , cells[index_to])->data;
            add_node_to_frequency_list
            (from_node, to_node, markov_chain);
        }
        else
        {
            for (int j = 1; j <= DICE_MAX; j++)
            {
                index_to = ((Cell*) (from_node->data))->number + j - 1;
                if (index_to >= BOARD_SIZE)
                {
                    break;
                }
                to_node = get_node_from_database
                        (markov_chain,
                         cells[index_to])->data;
                int  res = add_node_to_frequency_list
                        (from_node,
                         to_node, markov_chain);
                if(res==EXIT_FAILURE)
                {
                    return EXIT_FAILURE;
                }
            }
        }
    }
    for (size_t i = 0; i < BOARD_SIZE; i++)
    {
        free(cells[i]);
    }
    return EXIT_SUCCESS;
}

int comp_fun(void *first_data,void * second_data){
    const int *first_num = (const int *)first_data;
    const int *second_num = (const int *)second_data;
    return *first_num-*second_num;
}


void ptint_func(void *data){
    int * d = (int *)data;
    for (size_t i=0; i<NUM_OF_TRANSITIONS; i++){
        if (transitions[i][HEAD] == *d){
            if (transitions[i][TAIL] < transitions[i][HEAD]){
                fprintf
                (stdout," [%d] -snake to->", *d);
                return;
            } else{
                fprintf
                (stdout," [%d] -ladder to->", *d);
                return;
            }
        }
    }
    if (*d == BOARD_SIZE){
        fprintf(stdout, " [%d]",*d);
        return;
    }
    fprintf(stdout, " [%d] ->",*d);
}


void * copy_func(void * data){
    const int * num = (const int *)data;
    int *dup = (int *)malloc(sizeof(int));
    if (dup != NULL) {
        *dup = *num;
    }
    return  dup;
}


void free_data(void *data){
    free((int *)data);
}


bool is_last(void * data){
    int * num = (int *) data;
    return (*num == BOARD_SIZE);
}


/**
 * @param argc num of arguments
 * @param argv 1) Seed
 *             2) Number of sentences to generate
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int main(int argc, char *argv[]) {
    int check_args = is_right_num_args(argc);
    if (check_args == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }
    LinkedList *list = (LinkedList *) malloc(sizeof(LinkedList));
    if (list == NULL) {
        fprintf(stdout, ALLOCATION_ERROR_MASSAGE);
        return EXIT_FAILURE;
    }
    list->first = NULL;
    list->last = NULL;
    list->size = 0;
    MarkovChain *markov_chain =
            (MarkovChain *) malloc(sizeof(MarkovChain));
    if (markov_chain == NULL) {
        fprintf(stdout, ALLOCATION_ERROR_MASSAGE);
        return EXIT_FAILURE;
    }
    markov_chain->database = list;
    markov_chain->copy_func = copy_func;
    markov_chain->comp_func = comp_fun;
    markov_chain->is_last = is_last;
    markov_chain->print_func = ptint_func;
    markov_chain->free_data = free_data;
    long seed = strtol(argv[1], NULL, BASE_TEN);
    srand(seed);
    int make_the_chain = fill_database_snakes(markov_chain);
    if (make_the_chain == EXIT_FAILURE){
        return EXIT_FAILURE;
    }
    long max_paths = strtol(argv[2], NULL, BASE_TEN);
    int curr_walk = CURR_WALK;
    while (curr_walk <= max_paths){
        MarkovNode * first_node = markov_chain->database->first->data;
        fprintf(stdout,"Random Walk %d: [%d] ->"
                ,curr_walk, *(int *)first_node->data);
        //MarkovNode * first_node = get_first_random_node(markov_chain);
        //markov_chain->print_func(first_node->data);
        generate_random_sequence(markov_chain,first_node
                                 ,MAX_GENERATION_LENGTH);
        curr_walk++;
        fprintf(stdout,"\n");
    }
    free_markov_chain(&markov_chain);
    return EXIT_SUCCESS;
}