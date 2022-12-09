#include <stdio.h>
#include <string.h>

#include "router.c"
#include "commands.c"
#include "dist_vec.c"



typedef struct {
    int router_writes[N_NEIGHBORS];
    int read_from_routers;
} shell_comm;



// This initializes the pipes between the routers and main
// shell_comm * init_shell_fds() {
//     // All routers 
//     // - write to same FD provided by main
//     // - read from individual fds

//     int main2router[2];
//     pipe(main2router); // TODO need to make sure to close the read end after the processes are created
//     shell_comm shell;
//     shell.read_from_routers = main2router[FD_IN]; // need to read data from each router

//     for (int r = 0; r < N_NEIGHBORS; r++) {
//         int router2main[2]; // this is for reading from the main processes
//         pipe(router2main);

//         shell.router_writes[r] = router2main[FD_OUT];
//         routers[r].from_main_read = router2main[FD_IN];
//         routers[r].to_main_write = main2router[FD_OUT];
//     }
// }

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

void print_help() {
    printf("I am helping!\n");
}
// Takes the input string and attempts to match a command to it for additional handling
// `routers` is a pointer to an array of routers
void match_cmd(shell_state * shell, char * cmd, char arguments[MAX_ARGS][MAX_ARG_LEN], int n_args) {
    // for (int arg = 0; arg < n_args; arg++) {
    //     printf("arg %d: %s\n", arg, arguments[arg]);
    // }

    // Use an else-if ladder to find the appropriate command string
    if (strcasecmp(cmd, "start") == 0) {
        if (shell->sim_active == 0) {
            shell->sim_active = 1;
            start_simulation(shell);
        } else {
            printf("Simulation is already active!");
        }
    } else if (strcasecmp(cmd, "help") == 0) {
        print_help();
    } else if (strcasecmp(cmd, "lr") == 0) { // Handle list routers
        printf("Handled lr!\n");
        for (int r = 0; r < N_NEIGHBORS; r++) {
            printf("%d ", shell->routers[r].id);
        }
        printf("\n");

    } else if (strcasecmp(cmd, "lw") == 0) { // Handle list weights of router
        printf("Handled lw!\n");
        if (shell->sim_active == 0) { // If this sim isn't active, just update the structs
            list_weights_no_sim(shell, arguments, n_args);
        } else {

        }

    }  else if (strcasecmp(cmd, "update") == 0) { // Handle update of weights of router
        printf("Handled update!\n");

    } else if (strcasecmp(cmd, "display") == 0) { // Display distance vector of router
        printf("Handled display!\n");

    } else if (strcasecmp(cmd, "n_messages") == 0) { // List the number of messages
        printf("Handled n_messages!\n");

    } else if (strcasecmp(cmd, "exit") == 0) { // Exit the program
        printf("Handled exit!\n");
        exit(0);
    } else {
        printf("Unknown command. Please try again.\n");
    }
}

void handle_input(shell_state * shell) {
    while (1) {
        // read the entire line
        printf("> ");
        char user_input[INPUT_SIZE];
        fgets(user_input, INPUT_SIZE, stdin);
        remove_newline(user_input);
        
        // Tokenize the input to get the first word
        char * pch;
        pch = strtok (user_input, " ");
        
        char cmd[MAX_CMD_LEN];
        char args[MAX_ARGS][MAX_ARG_LEN];

        // Get the first input (this is the command)
        if (pch != NULL) { // there should be a ptr and the command should be smaller than this length
            strncpy(cmd, pch, MAX_CMD_LEN);  
            pch = strtok (NULL, " ");
        } else {
            printf("Bad command input. Please try again.\n");
            continue;
        }

        // Get the remaining arguments
        int n_args = 0;
        while (pch != NULL && n_args < MAX_ARGS)
        {
            strncpy(args[n_args], pch, MAX_ARG_LEN);
            n_args++;
            pch = strtok (NULL, " ");
        }

        if (n_args > MAX_ARGS) {
            printf("Exceeded max argument length %d.\n", MAX_ARG_LEN);
            continue;
        }

        
        //&& strlen(pch) < MAX_CMD_LEN
        //remove_newline(pch);
        // strncpy(cmd, pch, MAX_CMD_LEN);
        // Parse the arguments into an array of integers


        // match the command =
        match_cmd(shell, cmd, args, n_args);
    }
}

int main(int argc, char **argv) {
    shell_state shell;
    shell.n_exchanges = 0; // program starts with 0 exchanges
    shell.sim_active = 0; // simulation is not active at start

    for (int r = 0; r < N_NEIGHBORS; r++) {
        shell.routers[r].id = r;
        init_weights(&shell.routers[r]);
    }

    handle_input(&shell);
}