#include "test_utils.h"

Point * get_test(int test_no, size_t * n_p) {
    Point * test;

    switch (test_no) {
        case 0:

        *n_p = 5;
        test = malloc(sizeof(Point) * *n_p);
        test[0].x = 4;
        test[1].x = 3;
        test[2].x = 2;
        test[3].x = 5;
        test[4].x = 1;

        for (size_t i=0; i<*n_p; i++) {
            test[i].y = 0;
            test[i].z = 0;
        }

        return test;

        case 1:
        *n_p = 5;
        test = malloc(sizeof(Point) * *n_p);
        test[0].x = 2;
        test[1].x = 3;
        test[2].x = 2;
        test[3].x = 1;
        test[4].x = 4;

        for (size_t i=0; i<*n_p; i++) {
            test[i].y = 0;
            test[i].z = 0;
        }

        return test;

        case 2:
        *n_p = 5;
        test = malloc(sizeof(Point) * *n_p);
        test[0].x = 3;
        test[1].x = 2;
        test[2].x = 3;
        test[3].x = 1;
        test[4].x = 4;

        for (size_t i=0; i<*n_p; i++) {
            test[i].y = 0;
            test[i].z = 0;
        }

        return test;

        case 3: // example worked by hand for use in test kd tree build
        *n_p = 15;
        test = malloc(sizeof(Point) * *n_p);
        test[0] = pt_build(38,9,5);
        test[1] = pt_build(3,36,29);
        test[2] = pt_build(23,36,8);
        test[3] = pt_build(29,28,39);
        test[4] = pt_build(5,37,39);
        test[5] = pt_build(12,26,9);
        test[6] = pt_build(13,8,3);
        test[7] = pt_build(9,12,24);
        test[8] = pt_build(19,21,11);
        test[9] = pt_build(34,17,17);
        test[10] = pt_build(8,38,23);
        test[11] = pt_build(8,23,27);
        test[12] = pt_build(11,8,7);
        test[13] = pt_build(9,33,39);
        test[14] = pt_build(24,32,17);

        return test;
    }
    
    printf("test_no=%d not found", test_no);
    return NULL;
}

void free_test(Point * test) {
    free(test);
}

void print_pt(Point pt) {
    printf("(%.2f,%.2f,%.2f)\n", pt.x, pt.y, pt.z);
}

void print_all_with_dim(Point * points, int n, unsigned int dim) {
    if (dim == 0) {
        for (int i=0; i<n; i++) {
            printf("%.2f, ", points[i].x);
        }
    } else if (dim == 1) {
        for (int i=0; i<n; i++) {
            printf("%.2f ", points[i].y);
        }
    } else if (dim == 2) {
        for (int i=0; i<n; i++) {
            printf("%.2f ", points[i].z);
        }
    } else {
        printf("dim=%d not valid\n", dim);
    }

    printf("\n");
}

void _kd_flatten_impl(KDNode ** out, KDNode * node, size_t * i_p);

/* flattens tree into an array in sorted order 
   array is malloced and must be freed before program exit */
KDNode ** kd_flatten(KDTree * tree) {
    if (!tree || !tree->root) {return NULL;}
    size_t i = 0; 
    size_t * i_p = &i;
    KDNode ** flattened = malloc(sizeof(KDNode *) * tree->size);

    _kd_flatten_impl(flattened, tree->root, i_p);

    return flattened;
}

void _kd_flatten_impl(KDNode ** out, KDNode * node, size_t * i_p) {
    if (!node) {return;}
    _kd_flatten_impl(out, node->left, i_p);
    out[*i_p] = node;
    *i_p += 1;
    _kd_flatten_impl(out, node->right, i_p);
}