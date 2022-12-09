#ifndef SHELL_H_
#define SHELL_H_

#define INPUT_SIZE 200
#define MAX_CMD_LEN 10
#define MAX_ARGS N_NEIGHBORS + 1
#define MAX_ARG_LEN 10 // max arg of 4 characters in length

#include "router.h"

typedef struct
{
    router_t routers[N_NEIGHBORS];
    int n_exchanges;
    int sim_active;
    int routers_writefd[N_NEIGHBORS]; // this is the fd for talking to each router
    int routers_readfd; // this is the fd for getting data from the routers (as a whole)
    int process_pids[N_NEIGHBORS];
} shell_state;

typedef enum
{
    ITERATION
} notification;

typedef struct
{
    notification notif;
} r2main_msg;

typedef enum
{
    LIST_WEIGHTS = 0,
    UPDATE = 1,
    DISPLAY = 2,
    EXIT = 3
} cmd_enum;
typedef struct
{
    cmd_enum command;
    int args[MAX_ARGS];
} main2r_msg;

main2r_msg new_main2r(cmd_enum command) {
    // Initialize arguments all to 0
    main2r_msg msg;
    for (int arg = 0; arg < MAX_ARGS; arg++) {
        msg.args[arg] = 0;
    }

    msg.command = command;
    return msg;
}
#endif