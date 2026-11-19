#include "partition.h"

enum comp {FAIL, LT, EQ, GT};

int qselect_easy_test_impl(int test_no, size_t * n_p, size_t k);

int qselect_easy_test(int test_no) {
    int trial_result = 0;
    int test_result = 1;

    size_t i = 0;
    size_t n = 0;

    printf("test qselect:\n");

    /* uses a do while because n needs to be set first, which is done
       in the qselect_easy_test_impl */
    do {
        trial_result = qselect_easy_test_impl(test_no, &n, i);
        printf("test %lu - %d\n\n", i, trial_result);
        test_result &= trial_result;
        i++;
    } while (i<n);

    printf("full test result: %d\n\n", test_result);

    return test_result;
}

// results verification test takes O(n) time
int qselect_easy_test_impl(int test_no, size_t * n_p, size_t k) {
    Point * test_pts = get_test(test_no, n_p);
    Point curr_point;

    curr_point = quick_select_k_point(test_pts, *n_p, k, 0);
    print_all_with_dim(test_pts, *n_p, 0);

    printf(" -> ");
    print_pt(curr_point);
    
    int test_passed = 1;
    
    for (int i=k-1; i>=0; i--) {
        test_passed &= compare_points(test_pts[i], curr_point, 0) == LT;
    }
        
    enum comp out;
    for (size_t i=k; i<*n_p; i++) {
        out = compare_points(test_pts[i], curr_point, 0);
        test_passed &= (out == EQ || out == GT);
    }

    free_test(test_pts);
    return test_passed;
}

void show_partition(int test_no) {
    printf("test partition:\n");
    size_t n;
    Point * test = get_test(test_no, &n);

    print_all_with_dim(test, n, 0);
    partition_points(test, 0, n-1, 0);
    print_all_with_dim(test, n, 0);

    free_test(test);
    printf("\n\n");
}

void show_set_pivot(int test_no) {
    printf("test set pivot:\n");
    size_t n;
    Point * test = get_test(test_no, &n);
    print_all_with_dim(test, n, 0);
    set_partition_pivot_to_last(test, 0, n-1, 0);
    print_all_with_dim(test, n, 0);
    free_test(test);
    printf("\n\n");
}