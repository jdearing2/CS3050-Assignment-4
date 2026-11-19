#ifndef CUBE_QUERY_H
#define CUBE_QUERY_H

#include "../a4.h"
#include "test_utils.h"

#include <stdlib.h>

typedef struct _CubeQuery {
    Point point;
    double r;
    size_t min_index;
    size_t * expected_out;
    size_t expected_n_found;
} CubeQuery;

int test_kd_cube_query(int test_no);
// // these functions don't need to be exposed
// CubeQuery * build_test_queries(int test_no, size_t * n_out);
// void free_test_queries(CubeQuery * test_queries, size_t n_test_queries);

#endif