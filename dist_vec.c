
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



int send_data(router_t *from_r, int to_id, const void *buf, size_t bytes)
{
    // If the same router, throw an error
    assert(from_r->id != to_id);
    return write(from_r->r_writefds[to_id], buf, bytes);
}

void notify_neighbors(router_t *r)
{
    // printf("notified neighbors\n");
    // notify all router neighbors
    for (int n = 0; n < N_NEIGHBORS; n++)
    {
        if (n != r->id) {// Don't want to notify itself (for many reasons, but partially bc it doesn't have a file descriptor)
            // Send the neighbors distance vectors to the other neighbors
            r2r_msg msg;
            msg.sender_id = r->id;
            // snprintf(msg.info, MAX_INFO_LEN, "sent from %d", r->id);

            // Only send router's cost to its neighbors, not whole vector
            int *r_neighbor_costs = r->cost[r->id];
            memcpy(msg.neighbor_costs, r_neighbor_costs, sizeof(int) * N_NEIGHBORS);

            send_data(r, n, &msg, sizeof(msg));
            // printf("notified %d w/ size %ld\n", n, sizeof(msg));
        }
    }
}

// This function calculates the new minimum distance vectors for a router
// Need to repeat this for every router to find the minimum 
int distance_vector(int router_id, int cost[N_NEIGHBORS][N_NEIGHBORS]) {
    int updated = 0;
    for (int end_node = 0; end_node < N_NEIGHBORS; end_node++)
    {
        if (router_id == end_node) continue; // we don't include same start/stop node since that is always 0
        
        int min_cost = MAX_WEIGHT;
        for (int inter = 0; inter < N_NEIGHBORS; inter++)
        {
            // if (router_id == inter) continue; // don't include x->x because also gives 0

            // If by going through an intermediate node is cheaper, update the cost
            if (cost[router_id][inter] + cost[inter][end_node] < min_cost) {
                min_cost = cost[router_id][inter] + cost[inter][end_node];
            }
        }

        // Update the cost vector with the new shortest path
        if (min_cost != cost[router_id][end_node]) {
            updated = 1;
        }
        cost[router_id][end_node] = min_cost;

    }

    return updated;
}
// Apply Belman-Ford to recompute the best estimate for the least cost path
// Returns 1 if an update was made

//---------------------- NOTE TO WEI WEI---------------------------
// It turns out I didn't understand the algorithm because I don't know how to
// incorporate the vector update into the calculations
// I tried following a youtube tutorial /explanation and I did not understand how
// they got the #s they did. How did the least cost path know to converge if the belman ford 
// had no relation to 
// see video https://www.youtube.com/watch?v=_lAJyA70Z-o
void recompute_est(router_t *r)
{
    // Compute the distance vector Dx(y) = min { C(x, v) + Dv(y)} for every neighbor (y) for a router (x).
    // C(x, v) represents cost from x to intermediary node v
    // Find the minimum cost and update the distance vector with the new weight

    // Start node is assumed to be current router
    // memcpy(r->cost, r->distances, sizeof(r->distances)); // make the least cost be the same as the distance

    // for (int router = 0; router < N_NEIGHBORS; router++) {
    // This means we updated our routers least cost path so we need to notify all the other rouers
    for (int i = 0; i < N_NEIGHBORS; i++) {
        if (distance_vector(i, r->cost) == 1) {
            printf("updated!\n");
            notify_neighbors(r);
        }
    }
    
    // }
    
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
            memcpy(r->cost[r->id], msg.args, sizeof(int) * N_NEIGHBORS);
            recompute_est(r);
            printf("Updated least cost!\n");
            notify_neighbors(r);
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
            printf("got signal!\n");
            if (read(r->r_readfd, &neighbor_msg, sizeof(neighbor_msg)) < 0) {
                printf("Error in reading from routers\n");
                exit(0);
            }
            // Update the distance matrix with the new weight from the neighbor
            memcpy(r->cost[neighbor_msg.sender_id], neighbor_msg.neighbor_costs, sizeof(neighbor_msg.neighbor_costs));
            recompute_est(r);
        }
        else
        {
            continue; // we're using a spinlock, which isn't great but not much else we can do since we don't want to block commands from main
        }

        // Recompute the shortest path and notify neighbors of changes if any were made

        // notify neighbors every 0.25 seconds
        
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
            close(shell->routers_readfd); // process doesn't need to read shell's stuff
            close(shell->routers_writefd[r]); // TODO valgrind doesn't like me getting rid of all the file descriptors for some reason...

            // close all other read fds present for routers
            for (int n = 0; n < N_NEIGHBORS; n++) {
                if (n != shell->routers[r].id) {
                    close(shell->routers[n].r_readfd);
                }
            }

            printf("Router (%d) was started!\n", r);
            router_main(&shell->routers[r]); // this loops forever until told by the shell to stop
        }

        // Close unused file descriptors for talking to shell (again in main)
        close(routers[r].shell_writefd); // other routers don't need this router's talking to shell pipes
        close(routers[r].shell_readfd);

        // Close unused file descriptors for talking from router to router
        // close(routers[r].r_readfd); // other routers don't need to read this one's messages
        
        shell->process_pids[r] = pid;
    }
    // we close all unused write descriptors for routers writing to one another
    for (int n = 1; n < N_NEIGHBORS; n++) { // there is no descriptor for n=0 r=0
        close(routers[0].r_writefds[n]);
    }

    return 0;
}
