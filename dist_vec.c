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
    int read_fds[N_NEIGHBORS];
    int write_fds[N_NEIGHBORS];
    int cost[N_NEIGHBORS][N_NEIGHBORS]; // If entry is NULL, assume infinite weight
} router_t;

// Print out the distance vector table
void display_router(router_t * r) {
    // print header
    printf("+--------+------+------+------+\n");
    printf("| Router | 0    | 1    | 2    |\n");
    printf("+--------+------+------+------+\n");

    // Print rows
    for (int row = 0; row < N_NEIGHBORS; row++) {
        printf("|   %-4d | %-4d | %-4d | %-4d |\n", row, r->cost[row][0], r->cost[row][1], r->cost[row][2]);
    }
    printf("+--------+------+------+------+\n");
}

void test_main() {
    router_t r;
    r.id = 0;

    int cost[3][3] = {{0, 0, 0}, {1, 1, 1}, {2, 2, 2}};
    memcpy(r.cost, cost, sizeof(cost));    

    display_router(&r);
}

int main() {
    test_main();

    return 0;
}


void real_main() {
    // 1. Parse the command line arguments for weights
    // 2. Initialize 3 pipes and routers. Pass fds to each router
    // int r1_fd[2]; = pipe();
    // int r2_fd[2]; = pipe();
    // int r3_fd[2]; = pipe();

    router_t routers[N_NEIGHBORS];
    for (int r = 0; r < N_NEIGHBORS; r++) {
        // initialize each router struct and include fds to all the other routers
    }

    // For each router, fork
    int proc_pids[N_NEIGHBORS];
    for (int r = 0; r < N_NEIGHBORS; r++) {
        int pid = fork();
        if (pid == 0) { // we are in child process
            // router_main();
            // exit process once done
        }
        proc_pids[r] = pid;
    }

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