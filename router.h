#ifndef ROUTER_H_
#define ROUTER_H_

#define N_NEIGHBORS 5

typedef struct
{
    unsigned int id;
    int read_fd;
    int write_fds[N_NEIGHBORS];
    int cost[N_NEIGHBORS][N_NEIGHBORS]; // If entry is NULL, assume infinite weight
} router_t;

#endif
