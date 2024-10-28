#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>

#define WIDTH 80
#define HEIGHT 25

int allocate_memory(int ***current_gen_ptr, int ***next_gen_ptr, int **current_gen_val, int **next_gen_val);
int greeting(int **current_gen_ptr);
void manually_input(int **current_gen_ptr);
void input(int **current_gen_ptr);
int game_proccess(int **current_gen_ptr, int **next_gen_ptr, int *generation, int *population);
int population_count(int **current_gen_ptr);
void render_graphic(int **current_gen_ptr, int speed, int generation, int population);
void evolution(int **current_gen_ptr, int **next_gen_ptr);
int check_neighbours(int **current_gen_ptr, int row, int column);
void swap_gens(int ***current_gen_ptr, int ***next_gen_ptr);
int is_current_eq_next(int **current_gen_ptr, int **next_gen_ptr);
void in_game_control(int *speed, int *local_error);
void error_message(int error);
void end_game(int error, int generation, int population);
void free_allocate_memory(int **current_gen_ptr, int **next_gen_ptr, int *current_gen_val, int *next_gen_val);

int main() {
    int **current_gen_ptr = NULL, **next_gen_ptr = NULL;
    int *current_gen_val = NULL, *next_gen_val = NULL;
    int error = allocate_memory(&current_gen_ptr, &next_gen_ptr, &current_gen_val, &next_gen_val);

    initscr();

    int start = greeting(current_gen_ptr);
    int generation = 0, population = 0;

    if (!error && start) {
        error = game_proccess(current_gen_ptr, next_gen_ptr, &generation, &population);
    }

    if (error == 1) {
        error_message(error);
    } else if (error == 2) {
        error_message(error);
    } else if (start == 0) {
        error_message(2);
    } else if (error == 3) {
        end_game(error, generation, population);
        error = 0;
    } else if (error == 4) {
        end_game(error, generation, population);
        error = 0;
    } else if (error == 5) {
        error_message(error);
    }

    getchar();

    free_allocate_memory(current_gen_ptr, next_gen_ptr, current_gen_val, next_gen_val);

    endwin();

    return error;
}

int allocate_memory(int ***current_gen_ptr, int ***next_gen_ptr, int **current_gen_val, int **next_gen_val) {
    int local_error = 0;

    *current_gen_ptr = calloc(HEIGHT, sizeof(int *));
    *next_gen_ptr = calloc(HEIGHT, sizeof(int *));

    if (*current_gen_ptr == NULL || *next_gen_ptr == NULL) {
        local_error = 1;
    } else {
        *current_gen_val = calloc(HEIGHT * WIDTH, sizeof(int));
        *next_gen_val = calloc(HEIGHT * WIDTH, sizeof(int));
        for (int i = 0; i < HEIGHT; i++) {
            (*current_gen_ptr)[i] = *current_gen_val + WIDTH * i;
            (*next_gen_ptr)[i] = *next_gen_val + WIDTH * i;
        }
    }

    return local_error;
}

int greeting(int **current_gen_ptr) {
    int start = 1;

    clear();

    printw("Welcome to the Conway's Game of Life.\n\n");
    printw("Choose type of input to start the game:\n");
    printw("1. Input the initial state manually.\n");

    refresh();

    char c = getchar();

    while (!(c == '1') && !(c == 'q')) {
        c = getchar();
    }

    if (c == 'q') {
        start = 0;
    } else {
        manually_input(current_gen_ptr);
    }

    return start;
}

void manually_input(int **current_gen_ptr) {
    clear();
    printw("Write inittial state 80x25 and press any button:\n\n");
    refresh();

    input(current_gen_ptr);
}

void input(int **current_gen_ptr) {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (scanf("%1d", *(current_gen_ptr + i) + j) != 1 ||
                (current_gen_ptr[i][j] != 0 && current_gen_ptr[i][j] != 1)) {
                break;
            }
        }
    }
}

int game_proccess(int **current_gen_ptr, int **next_gen_ptr, int *generation, int *population) {
    int local_error = 0, speed = 100000, stagnation = 0;

    *population = population_count(current_gen_ptr);

    while (!local_error && !stagnation && *population > 0) {
        if (freopen("/dev/tty", "r", stdin) != NULL) {
            cbreak();
            nodelay(initscr(), 1);
            noecho();

            *population = population_count(current_gen_ptr);

            render_graphic(current_gen_ptr, speed, *generation, *population);
            evolution(current_gen_ptr, next_gen_ptr);
            swap_gens(&current_gen_ptr, &next_gen_ptr);
            stagnation = is_current_eq_next(current_gen_ptr, next_gen_ptr);

            *generation += 1;

            in_game_control(&speed, &local_error);

            if (stagnation == 1) {
                generation -= 1;
                local_error = 3;
            } else if (*population == 0) {
                local_error = 4;
            }
        } else {
            local_error = 5;
        }
    }

    return local_error;
}

int population_count(int **current_gen_ptr) {
    int count = 0;
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (current_gen_ptr[i][j] == 1) {
                count += 1;
            }
        }
    }
    return count;
}

void render_graphic(int **current_gen_ptr, int speed, int generation, int population) {
    clear();

    printw("Current speed %d\n", speed);
    printw("Generations %d\n", generation);
    printw("Population %d\n", population);
    printw("##################################################################################\n");

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = -1; j <= WIDTH; j++) {
            if (j == -1 || j == WIDTH) {
                printw("#");
            } else if (current_gen_ptr[i][j] == 1) {
                printw("o");
            } else {
                printw(" ");
            }
        }
        printw("\n");
    }

    printw("##################################################################################\n");
    printw("#                                       #                                        #\n");
    printw("#     a - to decrease speed of game     #     d - to increase speed of game      #\n");
    printw("#                                       #                                        #\n");
    printw("##################################################################################\n");
    printw("#                                                                                #\n");
    printw("#                           1, 2, 3 - to choose speed                            #\n");
    printw("#                                                                                #\n");
    printw("#                              q - to quit the game                              #\n");
    printw("#                                                                                #\n");
    printw("##################################################################################");

    refresh();
    usleep(speed);
}

void evolution(int **current_gen_ptr, int **next_gen_ptr) {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            int alive_neighbours = check_neighbours(current_gen_ptr, i, j);
            if (alive_neighbours == 3 && current_gen_ptr[i][j] == 0) {
                next_gen_ptr[i][j] = 1;
            } else if (current_gen_ptr[i][j] == 1 && alive_neighbours != 3 && alive_neighbours != 2) {
                next_gen_ptr[i][j] = 0;
            } else {
                next_gen_ptr[i][j] = current_gen_ptr[i][j];
            }
        }
    }
}

int check_neighbours(int **current_gen_ptr, int row, int column) {
    int alive_neighbours = 0;

    for (int i = row - 1; i <= row + 1; i++) {
        for (int j = column - 1; j <= column + 1; j++) {
            if (i != row || j != column) {
                int neighbour_row = (i + HEIGHT) % HEIGHT;
                int neighbour_col = (j + WIDTH) % WIDTH;
                alive_neighbours += current_gen_ptr[neighbour_row][neighbour_col];
            }
        }
    }

    return alive_neighbours;
}

void swap_gens(int ***current_gen_ptr, int ***next_gen_ptr) {
    int **temp = *current_gen_ptr;
    *current_gen_ptr = *next_gen_ptr;
    *next_gen_ptr = temp;
}

int is_current_eq_next(int **current_gen_ptr, int **next_gen_ptr) {
    int stagnation = 0;

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            if (current_gen_ptr[i][j] == next_gen_ptr[i][j]) {
                stagnation += 1;
            }
        }
    }

    return stagnation == HEIGHT * WIDTH ? 1 : 0;
}

void in_game_control(int *speed, int *local_error) {
    int speed_step = 10000, static_speed_1 = 50000, static_speed_2 = 150000, static_speed_3 = 10000;

    char key = getch();

    fflush(stdout);

    if (key == 'q') {
        *local_error = 2;
    } else if (key == 'd' && *speed > 10000) {
        *speed -= speed_step;
    } else if (key == 'a') {
        *speed += speed_step;
    } else if (key == '1') {
        *speed = static_speed_1;
    } else if (key == '2') {
        *speed = static_speed_2;
    } else if (key == '3') {
        *speed = static_speed_3;
    }
}

void error_message(int error) {
    clear();

    if (error == 1) {
        printw("Error number 1: Can't allocate memory for game.");
    } else if (error == 2) {
        printw("Game has been canceled!");
    } else if (error == 5) {
        printw("Unable to redirecty stdin from file");
    }

    refresh();
}

void end_game(int error, int generation, int population) {
    clear();
    if (error == 3) {
        printw("Game is over. There is stagnation of colony.\n");
        printw("Total Generations - %d\n", generation);
        printw("Last Populations - %d\n", population);
    } else if (error == 4) {
        printw("Game is over. No one is alive.\n");
        printw("Total Generations - %d\n", generation);
    }

    refresh();
}

void free_allocate_memory(int **current_gen_ptr, int **next_gen_ptr, int *current_gen_val,
                          int *next_gen_val) {
    free(current_gen_val);
    free(current_gen_ptr);
    free(next_gen_val);
    free(next_gen_ptr);
}