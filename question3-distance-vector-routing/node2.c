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
} node2_table;

int link_costs_node2[4] = { 3, 1, 0, 2 };
struct rtpkt outgoing_packets2[4];
int shortest_paths_node2[4];

int lesser_of_two_2(int x, int y) {
    return (x < y) ? x : y;
}

int min_from_array_2(int values[]) {
    return lesser_of_two_2(lesser_of_two_2(lesser_of_two_2(values[0], values[1]), values[2]), values[3]);
}

void update_shortest_paths_2() {
    for (int i = 0; i < 4; i++) {
        shortest_paths_node2[i] = min_from_array_2(node2_table.matrix[i]);
    }
}

void broadcast_packets_2() {
    for (int i = 0; i < 4; i++) {
        outgoing_packets2[i].sourceid = 2;
        outgoing_packets2[i].destid = i;
        memcpy(outgoing_packets2[i].mincost, shortest_paths_node2, sizeof(shortest_paths_node2));
    }

    for (int i = 0; i < 4; i++) {
        if (i != 2) {
            tolayer2(outgoing_packets2[i]);
            printf("At time t=%.3f, node %d sends packet to node %d with: (%d  %d  %d  %d)\n",
                   clocktime, outgoing_packets2[i].sourceid, outgoing_packets2[i].destid,
                   outgoing_packets2[i].mincost[0], outgoing_packets2[i].mincost[1],
                   outgoing_packets2[i].mincost[2], outgoing_packets2[i].mincost[3]);
        }
    }
}

void check_and_send_update_2() {
    int prev_min_costs[4];
    memcpy(prev_min_costs, shortest_paths_node2, sizeof(shortest_paths_node2));

    int has_changed = 0;
    update_shortest_paths_2();

    for (int i = 0; i < 4; i++) {
        if (prev_min_costs[i] != shortest_paths_node2[i]) {
            has_changed = 1;
        }
    }

    if (has_changed) {
        broadcast_packets_2();
    } else {
        printf("\nMinimum cost didn't change. No new packets are sent\n");
    }
}

void rtinit2() {
    printf("rtinit2() is called at time t=: %0.3f\n", clocktime);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            node2_table.matrix[i][j] = (i == j) ? link_costs_node2[i] : INFINITY;
        }
    }

    printdt2(&node2_table);
    update_shortest_paths_2();
    broadcast_packets_2();
}

void rtupdate2(struct rtpkt *rcvdpkt) {
    int src = rcvdpkt->sourceid;
    int received_costs[4];

    for (int i = 0; i < 4; i++) {
        received_costs[i] = rcvdpkt->mincost[i];
    }

    printf("rtupdate2() is called at time t=: %0.3f as node %d sent a pkt with (%d  %d  %d  %d)\n",
           clocktime, src,
           received_costs[0], received_costs[1], received_costs[2], received_costs[3]);

    for (int i = 0; i < 4; i++) {
        int potential_cost = node2_table.matrix[src][src] + received_costs[i];
        node2_table.matrix[i][src] = (potential_cost < INFINITY) ? potential_cost : INFINITY;
    }

    printdt2(&node2_table);
    check_and_send_update_2();
}

void printdt2(struct distance_table *dt) {
    printf("                via     \n");
    printf("   D2 |    0     1    3 \n");
    printf("  ----|-----------------\n");
    printf("     0|  %3d   %3d   %3d\n", dt->matrix[0][0], dt->matrix[0][1], dt->matrix[0][3]);
    printf("dest 1|  %3d   %3d   %3d\n", dt->matrix[1][0], dt->matrix[1][1], dt->matrix[1][3]);
    printf("     3|  %3d   %3d   %3d\n", dt->matrix[3][0], dt->matrix[3][1], dt->matrix[3][3]);
}

void printmincost2() {
    printf("Minimum cost from %d to other nodes are: %d %d %d %d\n", 2,
           shortest_paths_node2[0], shortest_paths_node2[1],
           shortest_paths_node2[2], shortest_paths_node2[3]);
}
