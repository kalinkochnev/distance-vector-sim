#include <stdlib.h>

#include "utils.c"
#include "shell.h"

// This function takes a reference to an argument and attempts to return a router pointer based on the inputted id
// Returns 1 if conversion is successful, otherwise 0
int get_router(shell_state * shell, router_t * router, char * arg) {
    int router_id;
    if (str2int(&router_id, arg, 10) == STR2INT_SUCCESS) {
        if (router_id < N_NEIGHBORS) {
            *router = shell->routers[router_id];
            return 1;
        } else {
            printf("Router out of bounds (max id %d)\n", N_NEIGHBORS);
        }
    } else {
        printf("Could not parse router ID. Did you input an integer?\n");
    }
    return -1;
}

void help() {

}

void list_weights_no_sim(shell_state * shell, char arguments[MAX_ARGS][MAX_ARG_LEN], int n_args ) {
    if (n_args != 1) {
        printf("Expected one argument.\n");
    }

    router_t r;
    if (get_router(shell, &r, arguments[0]) > 0) {
        for (int neighbor = 0; neighbor < N_NEIGHBORS; neighbor++) {
            if (neighbor == r.id) {
                continue;
            }

            printf("Weight %d->%d: %-4d\n", r.id, neighbor, r.cost[r.id][neighbor]);
        }
    }

}

void list_weights_w_sim() {

}