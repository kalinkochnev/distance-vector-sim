#include <stdio.h>
#include <string.h>

#include "router.h"

// #include "router.c"

#define INPUT_SIZE 200
#define MAX_CMD_LEN 10

typedef struct {
    int router_writes[N_NEIGHBORS];
    int read_from_routers;
} shell_comm;

// This initializes the pipes between the routers and main
shell_comm * init_shell_fds() {
    // All routers 
    // - write to same FD provided by main
    // - read from individual fds

    int main2router[2];
    pipe(main2router); // TODO need to make sure to close the read end after the processes are created
    shell_comm shell;
    shell.read_from_routers = main2router[FD_IN]; // need to read data from each router

    for (int r = 0; r < N_NEIGHBORS; r++) {
        int router2main[2]; // this is for reading from the main processes
        pipe(router2main);

        shell.router_writes[r] = router2main[FD_OUT];
        routers[r].from_main_read = router2main[FD_IN];
        routers[r].to_main_write = main2router[FD_OUT];
    }
}

// This closes the unused fds in main
void prune_shell_fds() {

}

// This function closes the remaining pipes
void close_shell_fds() {

}

void remove_newline(char * str) {
    char *pch = strstr(str, "\n");
    if(pch != NULL)
        strncpy(pch, "\0", 1);
}

// Takes the input string and attempts to match a command to it for additional handling
// `routers` is a pointer to an array of routers
void match_cmd(char * input, router_t routers[], int n_routers) {
    // Tokenize the input
    char * pch;
    pch = strtok (input, " ");
    
    // get the first thing tokenized (the command)
    char cmd_input[MAX_CMD_LEN];
    if (pch != NULL && strlen(pch) < MAX_CMD_LEN) { // there should be a ptr and the command should be smaller than this length
        remove_newline(pch);
        strncpy(cmd_input, pch, MAX_CMD_LEN);
    } else {
        printf("Bad command input. Please try again.\n");
        return;
    }

    // printf("recievd %s %d\n", cmd_input, strcmp(cmd_input, "exit"));

    // Use an else-if ladder to find the appropriate command string
    if (strcasecmp(cmd_input, "lr") == 0) { // Handle list routers
        printf("Handled lr!\n");
        for (int r = 0; r < n_routers; r++) {
            printf("%d ", routers[r].id);
        }
        printf("\n");

    } else if (strcasecmp(cmd_input, "lw") == 0) { // Handle list weights of router
        printf("Handled lw!\n");

    }  else if (strcasecmp(cmd_input, "update") == 0) { // Handle update of weights of router
        printf("Handled update!\n");

    } else if (strcasecmp(cmd_input, "display") == 0) { // Display distance vector of router
        printf("Handled display!\n");

    } else if (strcasecmp(cmd_input, "n_messages") == 0) { // List the number of messages
        printf("Handled n_messages!\n");

    } else if (strcasecmp(cmd_input, "exit") == 0) { // Exit the program
        printf("Handled exit!\n");
        exit(0);
    } else {
        printf("Unknown command. Please try again.\n");
    }
}

void handle_input(router_t routers[], int n_routers) {
    while (1) {
        // read the entire line
        printf("> ");
        char command[INPUT_SIZE];
        fgets(command, INPUT_SIZE, stdin);
        
        match_cmd(command, routers, n_routers);
    }
}