## Compilation commands:

Compile tests:
```
gcc -Werror -Wall -Wextra main.c a4.c tests/test_utils.c tests/partition.c tests/treebuild.c tests/cubequery.c -o bin/a.out
```

## TODOS

### Larger Goals

* Finish the project; I need to implement everything in a4_api.h. So far, I can only build, free, and query k-d trees.
* I need to clean some things up. My tests aren't all totally transparent, some of them just printing a couple values without and labels. I need to clean those up.
* I need to organize a4.c. It's written in such a way that it compiles as it is but retains code blocks in comments for if it were compiled around a4_api.h, which the spec requires. I'll make a folder for my submission file which will have a4.c trimmed for how it's expected in its submission. Also, it's a mess in some ways right now and generally needs some cleaning.
* I may change Lomuto's partitioning out for Hoare's scheme. It's really not necessary right now, but does improve efficency on specific datasets that Lomuto often does with worst case complexity.
* I may make a makefile. This would be done once testing grows into greater scope, but should not be necessary considering how small scale the assignment is. Chances are though, I'll never need one.

### Smaller, specific goals

* Write more tests for kd_cube_query that ensure dimension restraints never skip a subtree it shouldn't by checking the wrong dimension.
* Rewrite kd_cube query to omit recursion; the recursion loop may be memory inefficient. I'll query an LLM to analyze how the code is processed by the compiler to see if the compiler optimizes the issue away. If it's fine, and I can get it to be more readable without omitting recursion, I'll keep the recursion.