
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

#include "shell.h"
#include "router.h"

// This randomly assigns costs for a particular router to its neighbors

void clean_main_router_fds(router_t *routers)
{
    // Close the descriptors related to router-to-router communication
    // close the read end for each router
    for (int r = 0; r < N_NEIGHBORS; r++)
    {
        close(routers[r].r_readfd);
    }

    // Close all the write ends from a single node (since it has references to all write ends including itself)
    for (int neighbor = 0; neighbor < N_NEIGHBORS; neighbor++)
    {
        close(routers[0].r_writefds[neighbor]);
    }

    // The follow descriptors that are closed are related to communication from the processes to the main function (for shell purposes)
    // We don't need
}

// shell_comm * initialize_routers(router_t *routers)
// {

//     

//     // INITIALIZE THE ROUTER TO MAIN COMMUNICATION FOR THE SHELL (2 way)

// }

int send_data(router_t *from_r, int to_id, const void *buf, size_t bytes)
{
    // If the same router, throw an error
    assert(from_r->id != to_id);
    return write(from_r->r_writefds[to_id], buf, bytes);
}

void notify_neighbors(router_t *r)
{
    // notify all router neighbors
    for (int n = 0; n < N_NEIGHBORS; n++)
    {
        if (n != r->id)
        { // Don't want to notify itself
            // Send the neighbors distance vectors to the other neighbors
            r2r_msg msg;
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
int recompute_est(router_t *r, r2r_msg *new_est)
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

void process_user_command(router_t *r)
{
    main2r_msg msg;
    if (read(r->shell_readfd, &msg, sizeof(main2r_msg)) > 0)
    {
        switch (msg.command)
        {
        case EXIT:
            printf("Closing router %d\n", r->id);
            close_router(r);
            exit(0);
            break;

        case DISPLAY:
            display_router(r);
            break;

        case UPDATE:
            break;

        case LIST_WEIGHTS:
            print_weights(r);
            break;
        default:
            break;
        }
    }
    else
    {
        printf("An error has occurred in reading the message from main!\n");
    }
}

void router_main(router_t *r)
{
    int start_time = time(NULL);
    notify_neighbors(r); // start by notifying everyone initially
    // time(NULL) - start_time < 5
    while (1)
    {
        // Check if we have any messages from the main thread for commands to run
        if (fd_ready(r->shell_readfd) == 1)
        {
            process_user_command(r);
        }

        // Check if we have messages from other routers
        r2r_msg neighbor_msg;
        // printf("%d waiting for msg of size: %ld\n", r->id, sizeof(neighbor_msg));

        if (fd_ready(r->r_readfd) == 1)
        {
            read(r->r_readfd, &neighbor_msg, sizeof(neighbor_msg));
        }
        else
        {
            continue; // we're using a spinlock, which isn't great but not much else we can do since we don't want to block commands from main
        }

        // Recompute the shortest path and notify neighbors of changes if any were made
        if (recompute_est(r, &neighbor_msg) == 1)
        { // And update was made
            printf("id: %d update made!\n", r->id);
            notify_neighbors(r);
        }
        else
        {
            // notify neighbors every 0.25 seconds
            usleep(10000);
            notify_neighbors(r);
        }
    }
    printf("done!\n");
    return;

    // Clean up write and read file descriptors
}

int start_simulation(shell_state *shell)
{
    router_t * routers = shell->routers;

    printf("Initializing router processes................\n");
    shell->sim_active = 1;

    // Initialize shell to router file descriptors
    int shell_fds[2];
    pipe(shell_fds);
    shell->routers_readfd = shell_fds[FD_IN];

    // INITIALIZE THE ROUTER TO ROUTER COMMUNICATION
    // For each router, pass the write ends of all the other routers
    // Each router has only one read end
    for (int r = 0; r < N_NEIGHBORS; r++)
    {
        int r_fd[2];
        pipe(r_fd);

        routers[r].r_readfd = r_fd[FD_IN];

        // give all neighbors the write end of the router's pipe (don't give it to itself)
        for (int neighbor = 0; neighbor < N_NEIGHBORS; neighbor++)
        {
            if (neighbor != routers[r].id) {
                routers[neighbor].r_writefds[r] = r_fd[FD_OUT]; 
            }
        }
    }

    // For each router, fork
    for (int r = 0; r < N_NEIGHBORS; r++)
    {
        // shell related file descriptors
        int router_fds[2];
        pipe(router_fds);

        routers[r].shell_writefd = shell_fds[FD_OUT]; // router talking to the shell
        routers[r].shell_readfd = router_fds[FD_IN]; // router getting messages from main
        shell->routers_writefd[r] = router_fds[FD_OUT]; // shell talking to router

        int pid = fork();

        if (pid == 0) // we are in child process
        {
            // Close unused file descriptors for talking to shell
            close(shell->routers_writefd[r]);
            close(shell->routers_readfd); // process doesn't need to read shell's stuff

            printf("Router (%d) was started!\n", r);
            router_main(&shell->routers[r]); // this loops forever until told by the shell to stop
        }

        // Close unused file descriptors for talking to shell (again in main)
        close(routers[r].shell_writefd); // other routers don't need this router's talking to shell pipes
        close(routers[r].shell_readfd);

        // Close unused file descriptors for talking from router to router
        close(routers[r].r_readfd); // other routers don't need to read this one's messages
        
        shell->process_pids[r] = pid;
    }
    // we close all unused write descriptors for routers writing to one another
    for (int n = 0; n < N_NEIGHBORS; n++) {
        close(routers[0].r_writefds[n]);
    }

    return 0;
}
