## Compilation commands:

Compile tests for all functions, even internal functions:
```
gcc -Werror -Wall -Wextra all_tests.c a4.c tests/test_utils.c tests/partition.c tests/treebuild.c tests/cubequery.c tests/test_graph_stats.c -o bin/all_tests.out
```

Compile tests for submissions functions:
```
gcc -Werror -Wall -Wextra submit_tests.c submit/a4.c -o bin/submit_tests.out
```

## TODOS

* Submit!