#ifndef ROUTER_H_
#define ROUTER_H_

#define N_NEIGHBORS 5
#define FD_IN 0
#define FD_OUT 1

typedef struct
{
    unsigned int id;
    int read_fd;
    int write_fds[N_NEIGHBORS];
    int cost[N_NEIGHBORS][N_NEIGHBORS]; // If entry is NULL, assume infinite weight
    int from_main_read;
    int to_main_write;
} router_t;

// This struct contains the file descriptors for communicating with the main function for 
typedef struct {
    int read_fd;
    int write_fd;
} router_main_comm;

typedef struct
{
    int sender_id;
    int neighbor_costs[N_NEIGHBORS];
    // char info[MAX_INFO_LEN];
} router_msg;

#endif
