#include <stdio.h>
#include "submit/a4_api.h"

#define N 27

int main() {
    freopen("log/out.txt", "a", stdout);
    freopen("log/err.txt", "a", stderr);

    printf("###   BEGIN TEST   ###\n\n");
    
    Point points[N] = {
        {-1,-1,-1},
        {-1,-1, 0},
        {-1,-1, 1},
        {-1, 0,-1},
        {-1, 0, 0},
        {-1, 0, 1},
        {-1, 1,-1},
        {-1, 1, 0},
        {-1, 1, 1},
        { 0,-1,-1},
        { 0,-1, 0},
        { 0,-1, 1},
        { 0, 0,-1},
        { 0, 0, 0},
        { 0, 0, 1},
        { 0, 1,-1},
        { 0, 1, 0},
        { 0, 1, 1},
        { 1,-1,-1},
        { 1,-1, 0},
        { 1,-1, 1},
        { 1, 0,-1},
        { 1, 0, 0},
        { 1, 0, 1},
        { 1, 1,-1},
        { 1, 1, 0},
        { 1, 1, 1}
    };

    KDTree * tree = kd_build(points, N);

    kd_free(tree);

    GraphStats gs[5];
    gs[0] = graph_stats(points, N, 0.1);
    gs[1] = graph_stats(points, N, 0.5);
    gs[2] = graph_stats(points, N, 1.0);
    gs[3] = graph_stats(points, N, 2.0);
    gs[4] = graph_stats(points, N, 15.0);

    printf("Graph stats:\n");
    printf("Graph_no | Edges | Connected\n");
    for (int i=0; i<5; i++) {
        printf(
            "%8d | %5lu | %9d\n", i, 
            gs[i].edges, gs[i].connected
        );
    }
    printf("\n");
    
    printf("###    END TEST    ###\n");

}