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
} node0_table;

int link_costs_node0[4] = { 0, 1, 3, 7 };
struct rtpkt outgoing_packets0[4];
int shortest_paths_node0[4];

int lesser_of_two_0(int x, int y) {
    return (x < y) ? x : y;
}

int min_from_array_0(int values[]) {
    return lesser_of_two_0(lesser_of_two_0(lesser_of_two_0(values[0], values[1]), values[2]), values[3]);
}

void update_shortest_paths_0() {
    for (int i = 0; i < 4; i++) {
        shortest_paths_node0[i] = min_from_array_0(node0_table.matrix[i]);
    }
}

void broadcast_packets_0() {
    for (int i = 0; i < 4; i++) {
        outgoing_packets0[i].sourceid = 0;
        outgoing_packets0[i].destid = i;
        memcpy(outgoing_packets0[i].mincost, shortest_paths_node0, sizeof(shortest_paths_node0));
    }

    for (int i = 0; i < 4; i++) {
        if (i != 0) {
            tolayer2(outgoing_packets0[i]);
            printf("At time t=%.3f, node %d sends packet to node %d with: (%d  %d  %d  %d)\n",
                   clocktime, outgoing_packets0[i].sourceid, outgoing_packets0[i].destid,
                   outgoing_packets0[i].mincost[0], outgoing_packets0[i].mincost[1],
                   outgoing_packets0[i].mincost[2], outgoing_packets0[i].mincost[3]);
        }
    }
}

void check_and_send_update_0() {
    int previous_costs[4];
    memcpy(previous_costs, shortest_paths_node0, sizeof(shortest_paths_node0));

    update_shortest_paths_0();

    int has_changed = 0;
    for (int i = 0; i < 4; i++) {
        if (previous_costs[i] != shortest_paths_node0[i]) {
            has_changed = 1;
        }
    }

    if (has_changed) {
        broadcast_packets_0();
    } else {
        printf("\nMinimum cost didn't change. No new packets are sent\n");
    }
}

void rtinit0() {
    printf("rtinit0() is called at time t=: %0.3f\n", clocktime);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            node0_table.matrix[i][j] = (i == j) ? link_costs_node0[i] : INFINITY;
        }
    }

    printdt0(&node0_table);
    update_shortest_paths_0();
    broadcast_packets_0();
}

void rtupdate0(struct rtpkt *rcvdpkt) {
    int src = rcvdpkt->sourceid;
    int received_costs[4];

    for (int i = 0; i < 4; i++) {
        received_costs[i] = rcvdpkt->mincost[i];
    }

    printf("rtupdate0() is called at time t=: %0.3f as node %d sent a pkt with (%d  %d  %d  %d)\n",
           clocktime, src,
           received_costs[0], received_costs[1], received_costs[2], received_costs[3]);

    for (int i = 0; i < 4; i++) {
        int possible_cost = node0_table.matrix[src][src] + received_costs[i];
        node0_table.matrix[i][src] = (possible_cost < INFINITY) ? possible_cost : INFINITY;
    }

    printdt0(&node0_table);
    check_and_send_update_0();
}

void printdt0(struct distance_table *dt) {
    printf("                via     \n");
    printf("   D0 |    1     2    3 \n");
    printf("  ----|-----------------\n");
    printf("     1|  %3d   %3d   %3d\n", dt->matrix[1][1], dt->matrix[1][2], dt->matrix[1][3]);
    printf("dest 2|  %3d   %3d   %3d\n", dt->matrix[2][1], dt->matrix[2][2], dt->matrix[2][3]);
    printf("     3|  %3d   %3d   %3d\n", dt->matrix[3][1], dt->matrix[3][2], dt->matrix[3][3]);
}

void printmincost0() {
    printf("Minimum cost from %d to other nodes are: %d %d %d %d\n", 0,
           shortest_paths_node0[0], shortest_paths_node0[1],
           shortest_paths_node0[2], shortest_paths_node0[3]);
}

void linkhandler0(int linkid, int newcost) {
    int previous_costs_via_link[4];
    for (int i = 0; i < 4; i++) {
        previous_costs_via_link[i] = node0_table.matrix[i][linkid] - node0_table.matrix[linkid][linkid];
    }

    for (int i = 0; i < 4; i++) {
        node0_table.matrix[i][linkid] = newcost + previous_costs_via_link[i];
    }

    printdt0(&node0_table);
    check_and_send_update_0();
}
