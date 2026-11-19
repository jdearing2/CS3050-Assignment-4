/*
This is used for testing and has all functions I may need to test
*/
#ifndef A4_H
#define A4_H

#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>

typedef struct {
    double x, y, z;
} Point;

typedef struct _MetaPoint {
    size_t index;
    Point data;
} MetaPoint;

typedef struct _KDNode {
    Point data;
    struct _KDNode * left;
    struct _KDNode * right;

    size_t index;
    size_t max_index;
} KDNode;

typedef struct KDTree {
    size_t size;
    KDNode * root;
} KDTree;

typedef struct {
    size_t edges;
    int connected;
} GraphStats;

KDTree *kd_build(const Point *points, size_t n);
int kd_insert(KDTree * tree, Point pt, size_t index);

size_t kd_cube_query(
    const KDTree *tree, 
    Point center, double r,
    size_t min_index, size_t *indices
);

void kd_free(KDTree *tree);

// implementation functions

Point pt_build(double x, double y, double z);

enum comp compare_points(Point pt1, Point pt2, unsigned int dim);

void set_partition_pivot_to_last(
    Point * points, 
    size_t first, 
    size_t last, 
    unsigned int dim
);

size_t partition_points(
    Point * points, size_t first, size_t last, 
    unsigned int dim
);

Point quick_select_k_point(
    Point * points, size_t n, unsigned int k, unsigned int dim
);

size_t partition_meta_points(
    MetaPoint * points, size_t first, size_t last, 
    unsigned int dim
);

MetaPoint quick_select_k_meta_point(
    MetaPoint * points, size_t n, unsigned int k, unsigned int dim
);

#endif