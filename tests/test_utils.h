#include "../a4.h"

#include <stdio.h>
#include <stdlib.h>

Point * get_test(int test_no, size_t * n_p);

void free_test(Point * test);

void print_pt(Point pt);

void print_all_with_dim(Point * points, int n, unsigned int dim);

KDNode ** kd_flatten(KDTree * tree);