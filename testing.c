#include <stdio.h>

// Apply Belman-Ford to recompute the best estimate for the least cost path
// Returns 1 if an update was made
void recompute_est(int id, int cost[3][3])
{
    // Compute the distance vector Dx(y) = min { C(x, v) + Dv(y)} for every neighbor (y) for a router (x).
    // C(x, v) represents cost from x to intermediary node v
    // Find the minimum cost and update the distance vector with the new weight

    // Start node is assumed to be current router
    // memcpy(r->cost, r->distances, sizeof(r->distances)); // make the least cost be the same as the distance

    // For each neighboring node (not including itself), compare the distance 
    // to some destination 
    // For each router, compute the distance vector
    for (int dest = 0; dest < 3; dest++) {
        if (id == dest) continue;
        printf("dest %d\n",dest);

        int min_cost = 9999;
        for (int inter = 0; inter < 3; inter++) {
            // if (id == inter) continue;
            printf("inter %d\n", inter);
            printf("cost %d\n", cost[id][inter] + cost[inter][dest]);

            // If by going through an intermediate node is cheaper, update the 
            // printf("router: %d  %d+%d cost: %d ij %d,%d\n", id, cost[id][inter], cost[inter][dest], cost[id][inter] + cost[inter][dest], inter, dest);
            if (cost[id][inter] + cost[inter][dest] < min_cost) {
                min_cost = cost[id][inter] + cost[inter][dest];
            }
        }

        
        printf("min %d\n", min_cost);
        cost[id][dest] = min_cost;
        printf("\n");
    }
        
}

int main() {
    // int costs[3][3] = {{0, 1, 50}, {1, 0, 2}, {50, 2, 0}};
    // int costs[3][3] = {{0, 7, 18}, {7, 0, 11}, {18, 11, 0}};
    // int costs[3][3] = {{999, 999, 999}, {4, 0, 1}, {10, 1, 0}};
    // int costs[3][3] = { {0, 1, 5}, {1, 0, 2}, {999, 999, 999}};
    // int costs[3][3] = { {0, 1, 5}, {999, 999, 999}, {999, 999, 999}};
    int costs[3][3] = { {0, 60, 50}, {4, 0,1}, {5, 1, 0}};

    recompute_est(0, costs);
    recompute_est(1, costs);
    recompute_est(2, costs);


    for (int i =0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            printf("%d ", costs[i][j]);
        }
        printf("\n");
    }
}