#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "router.h"

// Sets all weights to default MAX_WEIGHT. Then randomly generates weights from router to neighbor
void init_routers(router_t * routers)
{

    for (int r = 0; r < N_NEIGHBORS; r++) {
        routers[r].id = r;

        // Preset costs all to 9999
        for (int i = 0; i < N_NEIGHBORS; i++)
        {
            for (int j = 0; j < N_NEIGHBORS; j++)
            {
                routers[r].cost[i][j] = MAX_WEIGHT;
                // r->distances[i][j] = MAX_WEIGHT;
            }
        }
    }

    // Initialize the distance vector for a particular router (want it to be symmetric across routers)
    for (int r =0; r < N_NEIGHBORS; r++) {
        for (int neighbor = 0; neighbor < N_NEIGHBORS; neighbor++)
        {
            
            if (neighbor == r)
            {
                routers[r].cost[r][r] = 0; // 0 along diagonal
                // r->distances[r->id][r->id] = 0;
            }
            else
            {
                routers[r].cost[r][neighbor] = rand() / 100000000 + 1;
                routers[neighbor].cost[neighbor][r] = routers[r].cost[r][neighbor]; // make it symmetric
                // r->distances[r->id][neighbor] = r->cost[r->id][neighbor];
            }
        }
    }

    // Set random weights for router;
    // Make the cost matrix symmetrix

    
}

void table_divider()
{
    printf("+-----------------+");
    for (int i = 0; i < N_NEIGHBORS; i++)
    {
        printf("--------+");
    }
    printf("\n");
}
// Print out the distance vector table
void display_router(router_t *r)
{
    // LEAST cost table
    // print header
    table_divider();
    printf("| Min Cost (id %d) |", r->id);
    for (int i = 0; i < N_NEIGHBORS; i++)
    {
        printf(" %d      |", i);
    }
    printf("\n");
    table_divider();

    // Print rows
    for (int row = 0; row < N_NEIGHBORS; row++)
    {
        printf("|   %-4d          |", row);

        // Print columns
        for (int col = 0; col < N_NEIGHBORS; col++)
        {
            printf(" %-4d   |", r->cost[row][col]);
        }
        printf("\n");
    }
    table_divider();

    // DISTANCE table
    // print header
    // table_divider();
    // printf("| Distance (id %d) |", r->id);
    // for (int i = 0; i < N_NEIGHBORS; i++)
    // {
    //     printf(" %d      |", i);
    // }
    // printf("\n");
    // table_divider();

    // // Print rows
    // for (int row = 0; row < N_NEIGHBORS; row++)
    // {
    //     printf("|   %-4d          |", row);

    //     // Print columns
    //     for (int col = 0; col < N_NEIGHBORS; col++)
    //     {
    //         printf(" %-4d   |", r->distances[row][col]);
    //     }
    //     printf("\n");
    // }
    // table_divider();

}

void print_weights(router_t * r) {
    for (int neighbor = 0; neighbor < N_NEIGHBORS; neighbor++)
    {
        printf("Costs: %d->%d: %-4d\n", r->id, neighbor, r->cost[r->id][neighbor]);
        // printf("Distances: %d->%d: %-4d\n", r->id, neighbor, r->distances[r->id][neighbor]);
    }
}

// This function closes all the file descriptors associated with this router
void close_router(router_t *r)
{
    // Close the read end from other routers
    close(r->r_readfd);

    // Close all the write descriptors to other routers
    for (int i = 0; i < N_NEIGHBORS; i++)
    {
        if (i != r->id) { // router doesn't have file descriptor to itself
            close(r->r_writefds[i]);
        }
    }

    // Close file descriptors involving talking to main
    close(r->shell_readfd);
    close(r->shell_writefd);
}