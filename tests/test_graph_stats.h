#ifndef TEST_GRAPH_STATS_H
#define TEST_GRAPH_STATS_H

/* Runs every graph_stats test, printing PASS/FAIL and a description for
   each one. Returns the number of failed tests (0 = all passed).
   Contains no main(), so it can be linked into a combined test runner:

       failures += run_graph_stats_tests();
*/
int run_graph_stats_tests(void);

#endif