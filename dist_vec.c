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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define N_NEIGHBORS 3
#define FD_IN 0
#define FD_OUT 1

typedef struct {
    unsigned int id;
    int read_fd;
    int write_fds[N_NEIGHBORS];
    int cost[N_NEIGHBORS][N_NEIGHBORS]; // If entry is NULL, assume infinite weight
} router_t;

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

void init_weights(router_t * r) {
    memset(r->cost, 0, sizeof(int) * N_NEIGHBORS * N_NEIGHBORS);

    for (int neighbor = 0; neighbor < N_NEIGHBORS; neighbor++) {
        if (neighbor == r->id) {
            r->cost[r->id][r->id] = 0;
        } else {
            r->cost[r->id][neighbor] = rand() / 100000000;
        }
    }
}

void initialize_routers() {
    // For each router, pass the write ends of all the other routers
    // Each router has only one read end
    router_t routers[N_NEIGHBORS];
    for (int r = 0; r < N_NEIGHBORS; r++) {
        int r_fd[2];
        pipe(r_fd);

        // initialize fields
        routers[r].id = r;
        routers[r].read_fd = r_fd[FD_IN];
        routers[r].write_fds[r] = -1; // you shouldn't access -1 FD!!!! WARNING this is just a filler value
        init_weights(&routers[r]);

        // give all neighbors the write end of the router's pipe (don't give it to yourself)
        for (int neighbor = 0; neighbor < N_NEIGHBORS; neighbor++) {
            if (r == neighbor) { // don't assign write fd to itself
                continue;
            }

            routers[neighbor].write_fds[r] = r_fd[FD_OUT];
        }

        display_router(&routers[r]);
    }

}
void test_main() { 

}

int main() {
    test_main();

    return 0;
}

void real_main(int argc, int **argv) {
    int seed = 3100;
    srand(seed);

    // 1. Parse the command line arguments for weights

    // 2. Initialize 3 pipes and routers. Pass fds to each router
    initialize_routers();

    


    // For each router, fork
    // int proc_pids[N_NEIGHBORS];
    // for (int r = 0; r < N_NEIGHBORS; r++) {
    //     int pid = fork();
    //     if (pid == 0) { // we are in child process
    //         // router_main();
    //         // exit process once done
    //     }
    //     proc_pids[r] = pid;
    // }

    // Close all the unnecessary pipes in main since only the routers use them

    // handle control c https://stackoverflow.com/questions/4217037/catch-ctrl-c-in-c

    // Wait for processes to finish
    for (int r = 0; r < N_NEIGHBORS; r++) {
        // waitpid(proc_pids[r])
    }
}



int get_fd(router_t * from_r, router_t * to_r) {
    // If the same router, throw an error
    return -1;
}



void notify_neighbors(router_t * r) {
    // notify all router neighbors
    for (int n = 0; n < N_NEIGHBORS; n++) {
        if (n == r->id) { // Don't want to notify itself
            continue;
        }

        // TODO don't know if I'm supposed to send the whole table or just the row. I think just a row
        //send(stuff)
    }
}

// Apply Belman-Ford to recompute the best estimate for the distance vector 
void recompute_est() {

}

// TODO research ways to wait for any data from multiple file descriptors (select() or poll()??)
void wait_neighbors() {

}

void router_main(router_t * t) {
    //Init
    // Clean up file descriptors

    // 1. Wait for a message from a neighbor
    wait_neighbors();
    // 2. Recompute the estimate
    recompute_est();
    
    // 3. If a change was made, notify neighbors of the change
    notify_neighbors(t);
}