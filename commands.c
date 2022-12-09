#include <stdlib.h>

#include "utils.c"
#include "shell.h"

// This function takes a reference to an argument and attempts to return a router pointer based on the inputted id
// Returns 1 if conversion is successful, otherwise 0
int get_router(shell_state * shell, router_t ** router, char * arg) {
    int router_id;
    if (str2int(&router_id, arg, 10) == STR2INT_SUCCESS) {
        if (router_id < N_NEIGHBORS) {
            *router = &shell->routers[router_id];
            return 1;
        } else {
            printf("Router out of bounds (max id %d)\n", N_NEIGHBORS - 1);
        }
    } else {
        printf("Could not parse router ID. Did you input an integer?\n");
    }
    return -1;
}

void help() {
    
}

void display_router_no_sim(shell_state * shell, char arguments[MAX_ARGS][MAX_ARG_LEN], int n_args) {
    if (n_args != 1) {
        printf("Expected one argument.\n");
        return;
    }

    router_t * r = NULL;
    if (get_router(shell, &r, arguments[0]) > 0) {
        display_router(r);
    }
}

void list_weights_no_sim(shell_state * shell, char arguments[MAX_ARGS][MAX_ARG_LEN], int n_args) {
    if (n_args != 1) {
        printf("Expected one argument.\n");
        return;
    }

    router_t * r = NULL;
    if (get_router(shell, &r, arguments[0]) > 0) {
        for (int neighbor = 0; neighbor < N_NEIGHBORS; neighbor++) {
            if (neighbor == r->id) {
                continue;
            }

            printf("Weight %d->%d: %-4d\n", r->id, neighbor, r->cost[r->id][neighbor]);
        }
    }

}

void list_weights_w_sim() {

}

void update_weights_no_sim(shell_state * shell, char arguments[MAX_ARGS][MAX_ARG_LEN], int n_args) {
    if (n_args != N_NEIGHBORS + 1) {
        printf("Expected %d arguments.\n", N_NEIGHBORS);
        printf("update <router id> <w0> <w1> <w2> ... <wn>\n");
        return;
    }

    router_t * r = NULL;
    if (get_router(shell, &r, arguments[0]) > 0) {
        for (int arg = 1; arg < n_args; arg++) {
            // Attempt to parse each weight
            int weight;
            if (str2int(&weight, arguments[arg], 10) == STR2INT_SUCCESS) {
                r->cost[r->id][arg - 1] = weight;
            } else {
                printf("Could not parse edge weight. Did you input an integer?\n");
            }
        }
    }
}