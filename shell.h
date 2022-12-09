#ifndef SHELL_H_
#define SHELL_H_

#define INPUT_SIZE 200
#define MAX_CMD_LEN 10
#define MAX_ARGS N_NEIGHBORS+1
#define MAX_ARG_LEN 10 // max arg of 4 characters in length

#include "router.h"

typedef struct {
    router_t routers[N_NEIGHBORS];
    int n_exchanges;
    int sim_active;
} shell_state;

#endif