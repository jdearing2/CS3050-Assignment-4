#include "cubequery.h"

CubeQuery * build_test_queries(int test_no, size_t * n_queries);
void free_test_queries(CubeQuery * test_queries, size_t n_test_queries);

int _verify_cube_query(
    size_t n, size_t * out_indices, size_t n_found, 
    size_t * expected_out, size_t expected_n_found
);

int test_kd_cube_query(int test_no) {

    printf("test kd cube query:\n\n");

    Point * test_points = NULL;
    size_t n = 0;
    KDTree * kd_tree = NULL;
    size_t n_queries = 0;
    CubeQuery * cube_queries = NULL;
    size_t * out_indices = NULL;

    int setup_passed = 1;

    test_points = get_test(test_no, &n);
    if (!test_points) {setup_passed = 0; goto cleanup;}
    kd_tree = kd_build(test_points, n);
    if (!kd_tree) {setup_passed = 0; goto cleanup;}
    cube_queries = build_test_queries(test_no, &n_queries);
    if (!cube_queries) {setup_passed = 0; goto cleanup;}
    out_indices = calloc(n, sizeof(size_t));
    if (!out_indices) {setup_passed = 0; goto cleanup;}
    
    int test_result = 1;
    int trial_result = 1;
    CubeQuery curr;
    size_t n_found;

    for (size_t i=0; i<n_queries; i++) {
        trial_result = 1;
        for (size_t i=0; i<n; i++) {out_indices[i] = 0;}

        curr = cube_queries[i];
        n_found = kd_cube_query(
            kd_tree, curr.point, curr.r, curr.min_index, out_indices
        );

        printf("trial %lu:\n", i);

        trial_result = _verify_cube_query(
            n, out_indices, n_found, curr.expected_out, 
            curr.expected_n_found
        );

        printf("trial result=%d\n\n", trial_result);
        test_result &= trial_result;
    }

    printf("test result=%d\n\n", test_result);

    cleanup:
    free_test(test_points);
    kd_free(kd_tree);
    free_test_queries(cube_queries, n_queries);
    free(out_indices);

    if (!setup_passed) {
        printf(
            "No cube query test found, or malloc failed."
            " Returning a negative.\n"
        );
    }

    return test_result;
}

int _verify_cube_query(
    size_t n, size_t * out, size_t n_found, 
    size_t * expected_out, size_t expected_n_found
) {
    int trial_result = (n_found == expected_n_found);
    printf(
        "found %lu out of expected %lu\n", 
        n_found, expected_n_found
    );

    int found;
    size_t query;
    for (size_t i=0; i<expected_n_found; i++) {
        query = expected_out[i];

        found = 0;
        for (size_t j=0; j<n; j++) {
            if (out[j] == query) {
                found = 1;
                break;
            }
        }

        if (!found) {
            printf("node with index %lu not found\n", query);
        }
        trial_result &= found;
    }

    return trial_result;
}

CubeQuery _build_query(
    Point point, double r, size_t min_index, 
    size_t * expected_out, size_t expected_n_found
);

CubeQuery * build_test_queries(int test_no, size_t * n_queries_p) {
    size_t n;
    Point center;
    double r;
    size_t min_index;
    size_t * expected_out;
    size_t expected_n_found;
    CubeQuery * queries;
    
    switch (test_no) {
        case 3:
        n = 15; // total number of points in original structure
        *n_queries_p = 4;
        queries = malloc(sizeof(CubeQuery) * (*n_queries_p));
        if (!queries) {return NULL;}
        
        // Query 0 - no points
        center = pt_build(-1,-1,-1);
        r = 0.1;
        min_index = 0;
        expected_out = calloc(n, sizeof(size_t));
        // all expected out are zero because none should satisfy query
        expected_n_found = 0;
        queries[0] = _build_query(
            center, r, min_index, expected_out, expected_n_found
        );

        // Query 1 - all points
        center = pt_build(20,20,20);
        r = 20;
        min_index = 0;
        expected_out = calloc(n, sizeof(size_t));
        expected_out[0] = 5;
        expected_out[1] = 13;
        expected_out[2] = 7;
        expected_out[3] = 12;
        expected_out[4] = 11;
        expected_out[5] = 1;
        expected_out[6] = 10;
        expected_out[7] = 4;
        expected_out[8] = 8;
        expected_out[9] = 0;
        expected_out[10] = 6;
        expected_out[11] = 9;
        expected_out[12] = 14;
        expected_out[13] = 2;
        expected_out[14] = 3;

        expected_n_found = 15;
        queries[1] = _build_query(
            center, r, min_index, expected_out, expected_n_found
        );
        
        
        // Query 2 - half points, filtered by index
        center = pt_build(20,20,20);
        r = 20;
        min_index = 8;
        expected_out = calloc(n, sizeof(size_t));
        for (size_t i=0; i<n-min_index; i++) {
            expected_out[i] = i+min_index;
        }

        expected_n_found = 7;
        queries[2] = _build_query(
            center, r, min_index, expected_out, expected_n_found
        );
        
        // Query 3 - specific points; should prune branches during search
        center = pt_build(6,36,34);
        r = 5;
        min_index = 0;
        expected_out = calloc(n, sizeof(size_t));
        expected_out[0] = 13;
        expected_out[1] = 1;
        expected_out[2] = 4;

        expected_n_found = 3;
        queries[3] = _build_query(
            center, r, min_index, expected_out, expected_n_found
        );
        
        return queries;


        case 4:
        n = 48; // total number of points in original structure
        *n_queries_p = 1;
        queries = malloc(sizeof(CubeQuery) * (*n_queries_p));
        if (!queries) {return NULL;}
        center = pt_build(2.5, 2.5, 0);
        r = 2;
        min_index = 0;
        expected_out = calloc(n, sizeof(size_t));

        // bottom left 4x4 of points; I promise I understand this
        {
            size_t i=0;
            for (size_t j=0; j<n; j++) {
                if (j/4 % 2 == 0 && j<32) {
                    expected_out[i] = j;
                    i++;
                }
            }
        }

        expected_n_found = 16;
        queries[0] = _build_query(
            center, r, min_index, expected_out, expected_n_found
        );
        
        return queries; 


        default:
        printf("no query found for test_no=%i\n", test_no);
        return NULL;
    }
    
}

CubeQuery _build_query(
    Point point, double r, size_t min_index, 
    size_t * expected_out, size_t expected_n_found
) {
    CubeQuery query;
    query.point = point;
    query.r = r;
    query.min_index = min_index;
    query.expected_out = expected_out;
    query.expected_n_found = expected_n_found;
    return query;
}

void free_test_queries(CubeQuery * test_queries, size_t n_test_queries) {
    for (size_t i=0; i<n_test_queries; i++) {
        free(test_queries[i].expected_out);
    }

    free(test_queries);
}