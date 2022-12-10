#ifndef ROUTER_H_
#define ROUTER_H_

#define N_NEIGHBORS 3
#define MAX_WEIGHT 9999
#define FD_IN 0
#define FD_OUT 1

typedef struct
{
    unsigned int id;
    int r_readfd; // this is the file descriptor to read from other routers
    int r_writefds[N_NEIGHBORS]; // these are the file descriptors to write to other routers
    int cost[N_NEIGHBORS][N_NEIGHBORS];
    int distances[N_NEIGHBORS][N_NEIGHBORS];
    int shell_readfd; // file descriptor to get data from main
    int shell_writefd; // file descriptor to write data to main
} router_t;

// This struct is used for communication from router to router
typedef struct
{
    int sender_id;
    int neighbor_costs[N_NEIGHBORS];
} r2r_msg;

#endif
