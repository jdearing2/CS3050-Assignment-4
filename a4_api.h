#ifndef A4_API_H
#define A4_API_H

#include <stddef.h>

typedef struct {
    double x, y, z;
} Point;

/* Opaque type: define struct KDTree in a4.c. */
typedef struct KDTree KDTree;

typedef struct {
    size_t edges;
    int connected;
} GraphStats;

/* Independent work: k-d tree and its helpers. */
KDTree *kd_build(const Point *points, size_t n);

/* Return original indices j >= min_index inside the closed cube
   centered at center with half-width r. Write each index once into
   indices, which has room for n entries. Output order is arbitrary. */
size_t kd_cube_query(const KDTree *tree, Point center, double r,
                     size_t min_index, size_t *indices);

void kd_free(KDTree *tree);

/* AI assistance permitted: graph construction and connectivity. */
GraphStats graph_stats(const Point *points, size_t n, double r);

#endif
