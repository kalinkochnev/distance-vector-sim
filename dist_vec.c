
/* INSTRUCTIONS
Use the below commands to run the algorithm. If you want to adjust the number of routers, modify the N_NEIGHBORS #define (warning not heavilty tested)
Please enter the input nicely since I have not handled every possible input error.

The program starts with randomly initialized weights (based on a seed)

COMMANDS
1. List commands
usage: `help`

2. Start simulation (uses randomly initialized weights)
usage: `start`

3. List router ids
usage: lr
> 0 1 2 3 4 5

4. List weights of router
usage: `lw <router id>`
> 10 12 13

5. Set weights of router
usage: `update <router id> <w1> <w2> .... <wn>`

6. Display router's current distance vector
usage: `display <router id>`

7. Messages exchanged since last update
usage: `n_messages`
> 10

8. Exit
usage: `exit`

*/
/* Guidelines
- Initialize 3 different routers with different weights (use a random seed)
- Implement the Distance Vector Algorithm

Notes:
- Every router can communicate to their neighbors and know the cost
- Whenever a weight is updated for a router, it updates its distance estimates and notifies the neighbors


Dx(y) = cost of least cost path from x to y
Let v be a neighbor
For each node y
Dx(y) = min_v { c(x, v) +                Dy(y)}
               ^- cost x to some neighbor v    ^-- min cost v to y



*/
#include <error.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "shell.c"
#include "router.h"

// #define MAX_INFO_LEN 50



void table_divider() {
    printf("+---------------+");
    for (int i = 0; i < N_NEIGHBORS; i++) {
        printf("------+");
    }
    printf("\n");
}
// Print out the distance vector table
void display_router(router_t *r)
{
    // print header
    table_divider();
    printf("| Router (id %d) |", r->id);
    for (int i = 0; i < N_NEIGHBORS; i++) {
        printf(" %d    |", i);
    }
    printf("\n");
    table_divider();

    // Print rows
    for (int row = 0; row < N_NEIGHBORS; row++)
    {
        printf("|   %-4d        |", row);

        // Print columns
        for (int col = 0; col < N_NEIGHBORS; col++)
        {
            printf(" %-4d |", r->cost[row][col]);
        }
        printf("\n");
    }
    table_divider();
}

// This randomly assigns costs for a particular router to its neighbors
void init_weights(router_t *r)
{
    for (int i = 0; i < N_NEIGHBORS; i++) {
        for (int j = 0; j < N_NEIGHBORS; j++) {
            r->cost[i][j] = 9999;
        }
    }

    for (int neighbor = 0; neighbor < N_NEIGHBORS; neighbor++)
    {
        if (neighbor == r->id)
        {
            r->cost[r->id][r->id] = 0;
        }
        else
        {
            r->cost[r->id][neighbor] = rand() / 100000000 + 1;
        }
    }
}

// This function closes all the file descriptors associated with this router
void close_router(router_t *r)
{
    // Close the read end
    close(r->read_fd);

    // Close all the write file descriptors
    for (int i = 0; i < N_NEIGHBORS; i++)
    {
        close(r->write_fds[i]);
    }
}

void clean_main_router_fds(router_t * routers) {
    // Close the descriptors related to router-to-router communication
    // close the read end for each router
    for (int r = 0; r < N_NEIGHBORS; r++)
    {
        close(routers[r].read_fd);
    }

    // Close all the write ends from a single node (since it has references to all write ends including itself)
    for (int neighbor = 0; neighbor < N_NEIGHBORS; neighbor++)
    {
        close(routers[0].write_fds[neighbor]);
    }

    // The follow descriptors that are closed are related to communication from the processes to the main function (for shell purposes)
    // We don't need 
}


shell_comm * initialize_routers(router_t *routers)
{
    // INITIALIZE THE ROUTER TO ROUTER COMMUNICATION
    // For each router, pass the write ends of all the other routers
    // Each router has only one read end
    for (int r = 0; r < N_NEIGHBORS; r++)
    {
        int r_fd[2];
        pipe(r_fd);

        // initialize fields
        routers[r].id = r;
        routers[r].read_fd = r_fd[FD_IN];
        init_weights(&routers[r]);

        // give all neighbors the write end of the router's pipe (don't give it to yourself)
        for (int neighbor = 0; neighbor < N_NEIGHBORS; neighbor++)
        {
            routers[neighbor].write_fds[r] = r_fd[FD_OUT]; // TODO do I need to use dup()?
        }

        display_router(&routers[r]);
    }

    // INITIALIZE THE ROUTER TO MAIN COMMUNICATION FOR THE SHELL (2 way)
    
}

int send_data(router_t *from_r, int to_id, const void *buf, size_t bytes)
{
    // If the same router, throw an error
    assert(from_r->id != to_id);
    return write(from_r->write_fds[to_id], buf, bytes);
}

void notify_neighbors(router_t *r)
{
    // notify all router neighbors
    for (int n = 0; n < N_NEIGHBORS; n++)
    {
        if (n != r->id)
        { // Don't want to notify itself
            // Send the neighbors distance vectors to the other neighbors
            router_msg msg;
            msg.sender_id = r->id;
            // snprintf(msg.info, MAX_INFO_LEN, "sent from %d", r->id);

            // Only send r's cost to its neighbors, not whole vector
            int *r_neighbor_costs = r->cost[r->id];
            memcpy(msg.neighbor_costs, r_neighbor_costs, sizeof(int) * N_NEIGHBORS);

            send_data(r, n, &msg, sizeof(msg));
            // printf("notified %d w/ size %ld\n", n, sizeof(msg));
        }
    }
}

// Apply Belman-Ford to recompute the best estimate for the least cost path
// Returns 1 if an update was made
int recompute_est(router_t *r, router_msg *new_est)
{
    // Update the cost matrix with the new cost from the neighbor
    memcpy(r->cost[new_est->sender_id], new_est->neighbor_costs, sizeof(new_est->neighbor_costs));

    // Compute the distance vector Dx(y) = min { C(x, v) + Dv(y)} for every neighbor (y) for a router (x).
    // C(x, v) represents cost from x to intermediary node v
    // Find the minimum cost and update the distance vector with the new weight

    // Start node is assumed to be current router
    int updated = 0;
    for (int end_node = 0; end_node < N_NEIGHBORS; end_node++)
    {
        int min_end_cost = r->cost[r->id][end_node];

        for (int n = 0; n < N_NEIGHBORS; n++)
        {
            int cost_rn = r->cost[r->id][n];          // cost from r to neighbor
            int cost_n_to_end = r->cost[n][end_node]; // cost from neighbor to

            if (cost_rn + cost_n_to_end < min_end_cost)
            {
                min_end_cost = cost_rn + cost_n_to_end;
                updated = 1;
            }
        }

        // Update the cost vector with the new shortest path
        r->cost[r->id][end_node] = min_end_cost;
    }
    return updated;
}

void router_main(router_t *r)
{
    int start_time = time(NULL);
    notify_neighbors(r); // start by notifying everyone initially
    //time(NULL) - start_time < 5
    while (1)
    {
        // Check if there is something available to read
        
        router_msg neighbor_msg;
        // printf("%d waiting for msg of size: %ld\n", r->id, sizeof(neighbor_msg));
        if (read(r->read_fd, &neighbor_msg, sizeof(neighbor_msg)) < 0) {
            break;
        }

        // Recompute the shortest path and notify neighbors of changes if any were made
        if (recompute_est(r, &neighbor_msg) == 1)
        { // And update was made
            printf("id: %d update made!\n", r->id);
            notify_neighbors(r);
        } else {
            // notify neighbors every 0.25 seconds
            usleep(10000);
            notify_neighbors(r);
        }

    }
    if (r->id == 1) {
        display_router(r);
    }
    printf("done!\n");
    return;

    // Clean up write and read file descriptors
}

int main(int argc, char **argv)
{
    int seed = 3100;
    srand(seed);

    // 1. Parse the command line arguments for weights

    // 2. Initialize 3 pipes and routers. Pass fds to each router
    router_t routers[N_NEIGHBORS];
    initialize_routers(routers);
    printf("Initializing routers................\n");

    // For each router, fork
    int proc_pids[N_NEIGHBORS];
    for (int r = 0; r < N_NEIGHBORS; r++)
    {
        int pid = fork();

        if (pid == 0)
        { // we are in child process
            // close all the read ends of the other routers inside of process
            for (int neighbor = 0; neighbor < N_NEIGHBORS; neighbor++)
            {
                if (neighbor != r)
                { // don't want to close router's own read end
                    close(routers[neighbor].read_fd);
                }
            }

            // Start the router main program
            printf("Router (%d) was started!\n", r);
            router_main(&routers[r]);

            // Clean up router file descriptors inside of process
            close_router(&routers[r]);
            exit(0); // exit once done
        }
        proc_pids[r] = pid;
    }

    // this closes all the pipes set up between routers in the main function
    clean_main_router_fds(routers);

    // // handle control c https://stackoverflow.com/questions/4217037/catch-ctrl-c-in-c

    sleep(1);
    printf("Enter `help` for a list of commands\n");
    handle_input(routers, N_NEIGHBORS);
    // // Wait for processes to finish
    // for (int r = 0; r < N_NEIGHBORS; r++)
    // {
    //     waitpid(proc_pids[r], NULL, 0);
    // }

    return 0;
}
