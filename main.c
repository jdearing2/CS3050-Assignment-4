/*
This file is not submitted and instead runs many tests described
by the libraries in the tests folder

These includes use a4.h, which exposed many more functions so each 
can be tested. The submission requires a4_api.h.
*/

#include <stdio.h>
#include "tests/partition.h"
#include "tests/treebuild.h"
#include "tests/cubequery.h"
#include "tests/test_graph_stats.h"

int main() {
    freopen("log/out.txt", "a", stdout);
    freopen("log/err.txt", "a", stderr);

    printf("###   BEGIN TEST   ###\n\n");
    
    show_set_pivot(0);

    show_partition(0);
    show_partition(1);
    show_partition(2);
    
    qselect_easy_test(0);

    test_kd_build(3);

    test_kd_cube_query(3);
    test_kd_cube_query(4);

    run_graph_stats_tests();

    printf("###    END TEST    ###\n");
}