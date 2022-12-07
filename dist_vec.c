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

#define N_NEIGHBORS 3
#define INFO_MSG_LEN 50
#define FD_IN 0
#define FD_OUT 1

typedef struct {
    unsigned int id;
    int read_fd;
    int write_fds[N_NEIGHBORS];
    int cost[N_NEIGHBORS][N_NEIGHBORS]; // If entry is NULL, assume infinite weight
} router_t;

typedef struct {
    int sender_id;
    int neighbor_costs[N_NEIGHBORS];
    char info[INFO_MSG_LEN];
} router_msg;

// Print out the distance vector table
void display_router(router_t * r) {
    // print header
    printf("+---------------+------+------+------+\n");
    printf("| Router (id %d) | 0    | 1    | 2    |\n", r->id);
    printf("+---------------+------+------+------+\n");

    // Print rows
    for (int row = 0; row < N_NEIGHBORS; row++) {
        printf("|   %-4d        |", row);

        // Print columns
        for (int col = 0; col < N_NEIGHBORS; col++) {
            printf(" %-4d |", r->cost[row][col]);
        }
        printf("\n");
    }
    printf("+---------------+------+------+------+\n");
}

// This randomly assigns costs for a particular router to its neighbors
void init_weights(router_t * r) {
    memset(r->cost, 0, sizeof(int) * N_NEIGHBORS * N_NEIGHBORS);

    for (int neighbor = 0; neighbor < N_NEIGHBORS; neighbor++) {
        if (neighbor == r->id) {
            r->cost[r->id][r->id] = __INT_MAX__;
        } else {
            r->cost[r->id][neighbor] = rand() / 100000000;
        }
    }
}

void initialize_routers(router_t * routers) {
    // For each router, pass the write ends of all the other routers
    // Each router has only one read end
    for (int r = 0; r < N_NEIGHBORS; r++) {
        int r_fd[2];
        pipe(r_fd);

        // initialize fields
        routers[r].id = r;
        routers[r].read_fd = r_fd[FD_IN];
        init_weights(&routers[r]);

        // give all neighbors the write end of the router's pipe (don't give it to yourself)
        for (int neighbor = 0; neighbor < N_NEIGHBORS; neighbor++) {
            routers[neighbor].write_fds[r] = r_fd[FD_OUT]; // TODO do I need to use dup()?
        }

        // display_router(&routers[r]);
    }
}





int send_data(router_t * from_r, int to_id, const void * buf, size_t bytes) {
    // If the same router, throw an error
    assert(from_r->id != to_id);
    return write(from_r->write_fds[to_id], buf, bytes);
}



void notify_neighbors(router_t * r) {
    // notify all router neighbors
    for (int n = 0; n < N_NEIGHBORS; n++) {
        if (n != r->id) { // Don't want to notify itself
            // Send the neighbors distance vectors to the other neighbors
            router_msg msg;
            msg.sender_id = r->id;
            snprintf(msg.info, INFO_MSG_LEN, "sent from %d", r->id);

            int * r_neighbor_costs = r->cost[r->id];
            memcpy(msg.neighbor_costs, r_neighbor_costs, sizeof(int) * N_NEIGHBORS);

            send_data(r, n, &msg, sizeof(msg));
            // printf("notified %d w/ size %ld\n", n, sizeof(msg));
        }
    }
}

// Apply Belman-Ford to recompute the best estimate for the distance vector 
void recompute_est() {

}

// TODO research ways to wait for any data from multiple file descriptors (select() or poll()??)
void wait_neighbors(router_t * t) {
    router_msg msg;
    read(t->read_fd, &msg, sizeof(msg));
}


void router_main(router_t * r) {
    //Init
    notify_neighbors(r);

    for (int n = 0; n < N_NEIGHBORS; n++) {
        router_msg neighbor_msg;
        // printf("%d waiting for msg of size: %ld\n", r->id, sizeof(neighbor_msg));
        read(r->read_fd, &neighbor_msg, sizeof(neighbor_msg));
        printf("%d recieved from %d! : \"%s\"\n", r->id, neighbor_msg.sender_id, neighbor_msg.info);
    }
    // 1. Wait for a message from a neighbor
    // wait_neighbors();

    // 2. Recompute the estimate
    // recompute_est();
    
    // 3. If a change was made, notify neighbors of the change
    // notify_neighbors(t);

    // Clean up write and read file descriptors
}

int main(int argc, char **argv) {
    int seed = 3100;
    srand(seed);

    // 1. Parse the command line arguments for weights

    // 2. Initialize 3 pipes and routers. Pass fds to each router
    router_t routers[N_NEIGHBORS];
    initialize_routers(routers);

    // For each router, fork
    int proc_pids[N_NEIGHBORS];
    for (int r = 0; r < N_NEIGHBORS; r++) {
        int pid = fork();

        if (pid == 0) { // we are in child process
        //     // close all the read ends of the other routers
        //     for (int neighbor = 0; neighbor < N_NEIGHBORS; neighbor++) {
        //         if (neighbor != r) { // don't want to close router's own read end
        //             close(routers[neighbor].read_fd);
        //         }
        //     }
            printf("%d process was started!\n", r);
            router_main(&routers[r]);
            exit(0);
        //     // exit process once done
        }
        proc_pids[r] = pid;
    }

    // Close all the unnecessary pipes in main since only the routers use them
    // for (int r = 0; r < N_NEIGHBORS; r++) {
    //     // close the read end for each router
    //     close(routers[r].read_fd);
    // }

    // // Close all the write ends from a single node (since it has references to all write ends including itself)
    // for (int neighbor = 0; neighbor < N_NEIGHBORS; neighbor++) {
    //     close(routers[0].write_fds[neighbor]);
    // }
    

    // // handle control c https://stackoverflow.com/questions/4217037/catch-ctrl-c-in-c

    // // Wait for processes to finish
    // for (int r = 0; r < N_NEIGHBORS; r++) {
    //     waitpid(proc_pids[r], NULL, 0);
    // }

    return 0;
}

