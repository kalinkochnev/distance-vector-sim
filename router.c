#include <stdlib.h>
#include "router.h"

// Sets all weights to default MAX_WEIGHT. Then randomly generates weights from router to neighbor
void init_weights(router_t *r)
{
    // Preset costs all to 9999
    for (int i = 0; i < N_NEIGHBORS; i++) {
        for (int j = 0; j < N_NEIGHBORS; j++) {
            r->cost[i][j] = MAX_WEIGHT;
        }
    }

    // Set random weights for router;
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
