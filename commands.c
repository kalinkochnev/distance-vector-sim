#include <stdlib.h>

#include "utils.c"
#include "shell.h"

// This function takes a reference to an argument and attempts to return a router pointer based on the inputted id
// Returns 1 if conversion is successful, otherwise 0
int get_router(shell_state *shell, router_t **router, char *arg)
{
    int router_id;
    if (str2int(&router_id, arg, 10) == STR2INT_SUCCESS)
    {
        if (router_id < N_NEIGHBORS)
        {
            *router = &shell->routers[router_id];
            return 1;
        }
        else
        {
            printf("Router out of bounds (max id %d)\n", N_NEIGHBORS - 1);
        }
    }
    else
    {
        printf("Could not parse router ID. Did you input an integer?\n");
    }
    return -1;
}

void help()
{
    // printf()
    printf("# of routers: %d\n\n", N_NEIGHBORS);
    printf("COMMANDS:\n");
    printf("1. List commands\n   usage: `help`\n\n");
    printf("2. Start simulation (uses randomly initialized weights)\n   usage: `start`\n\n");
    printf("3. List router IDs\n   usage: `lr`\n\n");
    printf("4. List weights/costs from router to its neighbors\n   usage: `lw <router id>`\n\n");
    printf("5. Set weights/costs for a router\n   usage: `update <router id> <w1> <w2> ... <wn>`\n\n");
    printf("6. Display router's current distance vector\n   usage: `display <router id>`\n\n");
    printf("7. Display messages exchanged since last update\n   usage: `n_messages`\n\n");
    printf("8. Exit\n   usage: `exit`\n\n");
}

void display_router_cmd(shell_state *shell, char arguments[MAX_ARGS][MAX_ARG_LEN], int n_args)
{
    if (n_args != 1)
    {
        printf("Expected one argument.\n");
        return;
    }

    router_t *r = NULL;
    if (get_router(shell, &r, arguments[0]) > 0)
    {
        if (shell->sim_active == 0) {
            display_router(r);
        } else {
            main2r_msg msg;
            msg.command = DISPLAY;
            write(shell->routers_writefd[r->id], &msg, sizeof(msg));
            wait_print();
        }
    }
}

void list_weights_cmd(shell_state *shell, char arguments[MAX_ARGS][MAX_ARG_LEN], int n_args)
{
    if (n_args != 1)
    {
        printf("Expected one argument.\n");
        return;
    }

    router_t *r = NULL;
    if (get_router(shell, &r, arguments[0]) > 0)
    {
        if (shell->sim_active == 0) {
            print_weights(r);
        } else {
            main2r_msg msg;
            msg.command = LIST_WEIGHTS;
            write(shell->routers_writefd[r->id], &msg, sizeof(msg));
            wait_print();
        }
    }
}

void update_weights_no_sim(shell_state *shell, char arguments[MAX_ARGS][MAX_ARG_LEN], int n_args)
{
    if (n_args != N_NEIGHBORS + 1)
    {
        printf("Expected %d arguments.\n", N_NEIGHBORS);
        printf("update <router id> <w0> <w1> <w2> ... <wn>\n");
        return;
    }

    router_t *r = NULL;
    if (get_router(shell, &r, arguments[0]) > 0)
    {
        for (int arg = 1; arg < n_args; arg++)
        {
            // Attempt to parse each weight
            int weight;
            if (str2int(&weight, arguments[arg], 10) == STR2INT_SUCCESS)
            {
                r->cost[r->id][arg - 1] = weight;
            }
            else
            {
                printf("Could not parse edge weight. Did you input an integer?\n");
            }
        }
    }
}

void exit_processes(shell_state *shell, char arguments[MAX_ARGS][MAX_ARG_LEN], int n_args) {
    if (n_args != 0)
    {
        printf("Expected no arguments.\n");
        return;
    }

    main2r_msg msg;
    msg.command = EXIT;
    for (int r = 0; r < N_NEIGHBORS; r++) {
        write(shell->routers_writefd[r], &msg, sizeof(msg));
    }
}