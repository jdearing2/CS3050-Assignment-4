#include "treebuild.h"

int _check_tree(KDTree * tree, int test_no, Point * test_pts);

int test_kd_build(int test_no) {
    printf("test kd build:\n");
    printf("test_no=%d\n\n", test_no);
    size_t n;
    int result = 1;
    Point * test_pts = get_test(test_no, &n);
    if (!test_pts) {return 0;}

    KDTree * tree = kd_build(test_pts, n);
    
    result = _check_tree(tree, test_no, test_pts);
    
    free_test(test_pts);
    kd_free(tree);

    printf("result=%d\n", result);
    printf("warning: result may contain a false positive if testing against"
        " a set without a hand-calculated solution. check which test_no's have"
        " completed solution in _check_tree in treebuild.c\n\n\n");
    return result;
}

int _check_tree(KDTree * tree, int test_no, Point * test_pts) {
    if (!tree || !test_pts) {return 0;}

    int result = 1;
    KDNode ** flattened_tree = kd_flatten(tree);
    if (!flattened_tree) {return 0;}

    size_t n = tree->size;

    switch (test_no) {
        case 3:
        
        size_t i_answer_key[] = {
            12, 7 , 11, 13, 10,
            1 , 4 , 5 , 6 , 0 , 
            9 , 8 , 2 , 14, 3
        };
        size_t max_i_answer_key[] = {
            12, 12, 11, 13, 10,
            10, 4 , 14, 6 , 9 , 
            9 , 14, 2 , 14, 3
        };

        printf("actual (index, min_index) - expected:\n");
        for (size_t i=0; i<n; i++) {
            printf("(%2lu, %2lu) - (%2lu, %2lu)\n", 
                flattened_tree[i]->index,
                flattened_tree[i]->max_index,
                i_answer_key[i],
                max_i_answer_key[i]
            );

            result &= (i_answer_key[i] == flattened_tree[i]->index);
            result &= (max_i_answer_key[i] == flattened_tree[i]->max_index);

        }
        printf("\n\n");
        
        break;
        
        default:
        printf("test_no=%d does not have known solution\n\n", test_no);
    }

    free(flattened_tree);
    
    /* cannot test if test_no does not have a known solution, so always
       returns positive in this case */
    return result;
}
