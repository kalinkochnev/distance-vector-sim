#include <stdio.h>
#include <string.h>

#include "router.c"
#include "commands.c"
#include "dist_vec.c"

// Closes all file descriptors used for talking to the processes
void close_shell_fds(shell_state * shell) {
    close(shell->routers_readfd);
    for (int r = 0; r < N_NEIGHBORS; r++) {
        close(shell->routers_writefd[r]);
    }
}

// Takes the input string and attempts to match a command to it for additional handling
// `routers` is a pointer to an array of routers
void match_cmd(shell_state *shell, char *cmd, char arguments[MAX_ARGS][MAX_ARG_LEN], int n_args)
{
    // for (int arg = 0; arg < n_args; arg++) {
    //     printf("arg %d: %s\n", arg, arguments[arg]);
    // }

    // Use an else-if ladder to find the appropriate command string
    if (strcasecmp(cmd, "start") == 0)
    {
        if (shell->sim_active == 0)
        {
            shell->sim_active = 1;
            start_simulation(shell);
            sleep(2);
        }
        else
        {
            printf("Simulation is already active!\n");
        }
    }
    else if (strcasecmp(cmd, "help") == 0)
    {
        help();
    }
    else if (strcasecmp(cmd, "lr") == 0)
    { // Handle list routers
        for (int r = 0; r < N_NEIGHBORS; r++)
        {
            printf("%d ", shell->routers[r].id);
        }
        printf("\n");
    }
    else if (strcasecmp(cmd, "lw") == 0)
    { // Handle list weights of router
        list_weights_cmd(shell, arguments, n_args);
        wait_print();
    }
    else if (strcasecmp(cmd, "update") == 0)
    { // Handle update of weights of router
        update_weights(shell, arguments, n_args);
    }
    else if (strcasecmp(cmd, "display") == 0)
    { // Display distance vector of router
        display_router_cmd(shell, arguments, n_args);
    }
    else if (strcasecmp(cmd, "n_messages") == 0)
    { // List the number of messages
        printf("Handled n_messages!\n");
    }
    else if (strcasecmp(cmd, "exit") == 0)
    { // Exit the program
        if (shell->sim_active == 1) {
            // Wait for processes to exit
            printf("Waiting for processes to exit...\n");
            exit_processes(shell, arguments, n_args);

            for (int r = 0; r < N_NEIGHBORS; r++) {
                waitpid(shell->process_pids[r], NULL, 0);
            }

            // clean up shell file descriptors
            close_shell_fds(shell);
        }        
        
        printf("Finished!\n");
        exit(0);
    }
    else
    {
        printf("Unknown command. Please try again.\n");
    }
}

void handle_input(shell_state *shell)
{
    while (1)
    {
        // read the entire line
        printf("> ");
        char user_input[INPUT_SIZE];
        fgets(user_input, INPUT_SIZE, stdin);
        remove_newline(user_input);

        // Tokenize the input to get the first word
        char *pch;
        pch = strtok(user_input, " ");

        char cmd[MAX_CMD_LEN];
        char args[MAX_ARGS][MAX_ARG_LEN];

        // Get the first input (this is the command)
        if (pch != NULL)
        { // there should be a ptr and the command should be smaller than this length
            strncpy(cmd, pch, MAX_CMD_LEN);
            pch = strtok(NULL, " ");
        }
        else
        {
            printf("Bad command input. Please try again.\n");
            continue;
        }

        // Get the remaining arguments
        int n_args = 0;
        while (pch != NULL && n_args < MAX_ARGS)
        {
            strncpy(args[n_args], pch, MAX_ARG_LEN);
            n_args++;
            pch = strtok(NULL, " ");
        }

        match_cmd(shell, cmd, args, n_args);
    }
}

void print_welcome() {
    printf("________  ___  ________  _______           ________  ________  ___  ___  _________  _______\n");
    printf("\\   __  \\|\\  \\|\\   __  \\|\\  ___ \\         |\\   __  \\|\\   __  \\|\\  \\|\\  \\|\\___   ___\\\\  ___ \\   \n");  
    printf("\\ \\  \\|\\  \\ \\  \\ \\  \\|\\  \\ \\   __/|        \\ \\  \\|\\  \\ \\  \\|\\  \\ \\  \\\\\\  \\|___ \\  \\_\\ \\   __/|    \n");
    printf(" \\ \\   ____\\ \\  \\ \\   ____\\ \\  \\_|/__       \\ \\   _  _\\ \\  \\\\\\  \\ \\  \\\\\\  \\   \\ \\  \\ \\ \\  \\_|/__  \n");
    printf("  \\ \\  \\___|\\ \\  \\ \\  \\___|\\ \\  \\_|\\ \\       \\ \\  \\\\  \\\\ \\  \\\\\\  \\ \\  \\\\\\  \\   \\ \\  \\ \\ \\  \\_|\\ \\ \n");
    printf("   \\ \\__\\    \\ \\__\\ \\__\\    \\ \\_______\\       \\ \\__\\\\ _\\\\ \\_______\\ \\_______\\   \\ \\__\\ \\ \\_______\\\n");
    printf("    \\|__|     \\|__|\\|__|     \\|_______|        \\|__|\\|__|\\|_______|\\|_______|    \\|__|  \\|_______|\n");

    printf("--------------------------------------------V0.0.1 2022--------------------------------------------\n");
    printf("author kalin kochnev \n");
    printf("Enter `help` to see a list of available commands\n");
}

int main(int argc, char **argv)
{
    print_welcome();
    shell_state shell;
    shell.n_exchanges = 0; // program starts with 0 exchanges
    shell.sim_active = 0;  // simulation is not active at start

    int seed = 3100;
    srand(seed);

    init_routers(shell.routers);


    handle_input(&shell);
}