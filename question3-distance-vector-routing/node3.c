#include <stdio.h>
#include <string.h>

extern struct rtpkt {
    int sourceid;
    int destid;
    int mincost[4];
};

extern int TRACE;
extern int YES;
extern int NO;
extern float clocktime;

#define INFINITY 999

struct distance_table {
    int matrix[4][4];
} node3_table;

int link_costs_node3[4] = { 7, 999, 2, 0 };
struct rtpkt outgoing_packets3[4];
int shortest_paths_node3[4];

int lesser_of_two_3(int x, int y) {
    return (x < y) ? x : y;
}

int min_from_array_3(int values[]) {
    return lesser_of_two_3(lesser_of_two_3(lesser_of_two_3(values[0], values[1]), values[2]), values[3]);
}

void update_shortest_paths_3() {
    for (int i = 0; i < 4; i++) {
        shortest_paths_node3[i] = min_from_array_3(node3_table.matrix[i]);
    }
}

void broadcast_packets_3() {
    for (int i = 0; i < 4; i++) {
        outgoing_packets3[i].sourceid = 3;
        outgoing_packets3[i].destid = i;
        memcpy(outgoing_packets3[i].mincost, shortest_paths_node3, sizeof(shortest_paths_node3));
    }

    for (int i = 0; i < 4; i++) {
        if (i != 3 && i != 1) {
            tolayer2(outgoing_packets3[i]);
            printf("At time t=%.3f, node %d sends packet to node %d with: (%d  %d  %d  %d)\n",
                   clocktime, outgoing_packets3[i].sourceid, outgoing_packets3[i].destid,
                   outgoing_packets3[i].mincost[0], outgoing_packets3[i].mincost[1],
                   outgoing_packets3[i].mincost[2], outgoing_packets3[i].mincost[3]);
        }
    }
}

void check_and_send_update_3() {
    int previous_costs[4];
    memcpy(previous_costs, shortest_paths_node3, sizeof(shortest_paths_node3));

    int has_changed = 0;
    update_shortest_paths_3();

    for (int i = 0; i < 4; i++) {
        if (previous_costs[i] != shortest_paths_node3[i]) {
            has_changed = 1;
        }
    }

    if (has_changed) {
        broadcast_packets_3();
    } else {
        printf("\nMinimum cost didn't change. No new packets are sent\n");
    }
}

void rtinit3() {
    printf("rtinit3() is called at time t=: %0.3f\n", clocktime);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            node3_table.matrix[i][j] = (i == j) ? link_costs_node3[i] : INFINITY;
        }
    }

    printdt3(&node3_table);
    update_shortest_paths_3();
    broadcast_packets_3();
}

void rtupdate3(struct rtpkt *rcvdpkt) {
    int src = rcvdpkt->sourceid;
    int received_costs[4];

    for (int i = 0; i < 4; i++) {
        received_costs[i] = rcvdpkt->mincost[i];
    }

    printf("rtupdate3() is called at time t=: %0.3f as node %d sent a pkt with (%d  %d  %d  %d)\n",
           clocktime, src,
           received_costs[0], received_costs[1], received_costs[2], received_costs[3]);

    for (int i = 0; i < 4; i++) {
        int potential_cost = node3_table.matrix[src][src] + received_costs[i];
        node3_table.matrix[i][src] = (potential_cost < INFINITY) ? potential_cost : INFINITY;
    }

    printdt3(&node3_table);
    check_and_send_update_3();
}

void printdt3(struct distance_table *dt) {
    printf("             via     \n");
    printf("   D3 |    0     2 \n");
    printf("  ----|-----------\n");
    printf("     0|  %3d   %3d\n", dt->matrix[0][0], dt->matrix[0][2]);
    printf("dest 1|  %3d   %3d\n", dt->matrix[1][0], dt->matrix[1][2]);
    printf("     2|  %3d   %3d\n", dt->matrix[2][0], dt->matrix[2][2]);
}

void printmincost3() {
    printf("Minimum cost from %d to other nodes are: %d %d %d %d\n", 3,
           shortest_paths_node3[0], shortest_paths_node3[1],
           shortest_paths_node3[2], shortest_paths_node3[3]);
}
