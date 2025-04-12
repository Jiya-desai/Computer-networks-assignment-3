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
} node1_table;

int link_costs_node1[4] = { 1, 0, 1, 999 };
struct rtpkt outgoing_packets1[4];
int shortest_paths_node1[4];

int lesser_of_two_1(int x, int y) {
    return (x < y) ? x : y;
}

int min_from_array_1(int costs[]) {
    return lesser_of_two_1(lesser_of_two_1(lesser_of_two_1(costs[0], costs[1]), costs[2]), costs[3]);
}

void update_shortest_paths_1() {
    for (int i = 0; i < 4; i++) {
        shortest_paths_node1[i] = min_from_array_1(node1_table.matrix[i]);
    }
}

void broadcast_packets_1() {
    for (int i = 0; i < 4; i++) {
        outgoing_packets1[i].sourceid = 1;
        outgoing_packets1[i].destid = i;
        memcpy(outgoing_packets1[i].mincost, shortest_paths_node1, sizeof(shortest_paths_node1));
    }

    for (int i = 0; i < 3; i++) {
        if (i != 1) {
            tolayer2(outgoing_packets1[i]);
            printf("At time t=%.3f, node %d sends packet to node %d with: (%d  %d  %d  %d)\n",
                   clocktime, outgoing_packets1[i].sourceid, outgoing_packets1[i].destid,
                   outgoing_packets1[i].mincost[0], outgoing_packets1[i].mincost[1],
                   outgoing_packets1[i].mincost[2], outgoing_packets1[i].mincost[3]);
        }
    }
}

void check_and_send_update_1() {
    int prev_min_costs[4];
    memcpy(prev_min_costs, shortest_paths_node1, sizeof(shortest_paths_node1));

    int has_changed = 0;
    update_shortest_paths_1();

    for (int i = 0; i < 4; i++) {
        if (prev_min_costs[i] != shortest_paths_node1[i]) {
            has_changed = 1;
        }
    }

    if (has_changed) {
        broadcast_packets_1();
    } else {
        printf("\nMinimum cost didn't change. No new packets are sent\n");
    }
}

void rtinit1() {
    printf("rtinit1() is called at time t=: %0.3f\n", clocktime);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            node1_table.matrix[i][j] = (i == j) ? link_costs_node1[i] : INFINITY;
        }
    }

    printdt1(&node1_table);
    update_shortest_paths_1();
    broadcast_packets_1();
}

void rtupdate1(struct rtpkt *packet) {
    int sender = packet->sourceid;
    int received_costs[4];

    for (int i = 0; i < 4; i++) {
        received_costs[i] = packet->mincost[i];
    }

    printf("rtupdate1() is called at time t=: %0.3f as node %d sent a pkt with (%d  %d  %d  %d)\n",
           clocktime, sender,
           received_costs[0], received_costs[1], received_costs[2], received_costs[3]);

    for (int i = 0; i < 4; i++) {
        int new_cost = node1_table.matrix[sender][sender] + received_costs[i];
        node1_table.matrix[i][sender] = (new_cost < INFINITY) ? new_cost : INFINITY;
    }

    printdt1(&node1_table);
    check_and_send_update_1();
}

void printdt1(struct distance_table *dt) {
    printf("             via   \n");
    printf("   D1 |    0     2 \n");
    printf("  ----|-----------\n");
    printf("     0|  %3d   %3d\n", dt->matrix[0][0], dt->matrix[0][2]);
    printf("dest 2|  %3d   %3d\n", dt->matrix[2][0], dt->matrix[2][2]);
    printf("     3|  %3d   %3d\n", dt->matrix[3][0], dt->matrix[3][2]);
}

void printmincost1() {
    printf("Minimum cost from %d to other nodes are: %d %d %d %d\n", 1,
           shortest_paths_node1[0], shortest_paths_node1[1],
           shortest_paths_node1[2], shortest_paths_node1[3]);
}

void linkhandler1(int linkid, int newcost) {
    int delta_costs[4];

    for (int i = 0; i < 4; i++) {
        delta_costs[i] = node1_table.matrix[i][linkid] - node1_table.matrix[linkid][linkid];
    }

    int updated_link_cost = newcost;

    for (int i = 0; i < 4; i++) {
        node1_table.matrix[i][linkid] = updated_link_cost + delta_costs[i];
    }

    printdt1(&node1_table);
    check_and_send_update_1();
}
