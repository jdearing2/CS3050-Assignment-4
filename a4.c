
#include "a4.h"

#include <stdlib.h>

enum comp {FAIL, LT, EQ, GT};

// typedef struct _MetaPoint {
//     size_t index;
//     Point data;
// } MetaPoint;

int kd_insert(KDTree * tree, Point pt, size_t index);
KDNode * kd_node_build(Point pt, size_t index);
int is_point_in_rect_prism(Point pt, Point min_vert, Point max_vert);
Point pt_build(double x, double y, double z);
enum comp compare_points(Point pt1, Point pt2, unsigned int dim);
size_t partition_points(
    Point * points, 
    size_t first, 
    size_t last, 
    unsigned int dim
);
size_t partition_meta_points(
    MetaPoint * points, 
    size_t first, 
    size_t last, 
    unsigned int dim
);
Point quick_select_k_point(
    Point * points, 
    size_t n, 
    unsigned int k, 
    unsigned int dim
);
MetaPoint quick_select_k_meta_point(
    MetaPoint * points,
    size_t n,
    unsigned int k,
    unsigned int dim
);
void set_partition_pivot_to_last(
    Point * points, 
    size_t first, 
    size_t last, 
    unsigned int dim
);
void meta_set_partition_pivot_to_last(
    MetaPoint * points, 
    size_t first, 
    size_t last, 
    unsigned int dim
);

/* original code for a k-d tree - not AI assisted in any way */

// typedef struct _KDNode {
//     Point data;
//     struct _KDNode * left;
//     struct _KDNode * right;

//     size_t index;
//     size_t min_index;
// } KDNode;

// typedef struct KDTree {
//     size_t size;
//     KDNode * root;
// } KDTree;

void kd_build_impl(KDTree * tree, MetaPoint * points, size_t n, unsigned int dim);

KDTree *kd_build(const Point *points, size_t n) {
    if (!points || n == 0) {return NULL;}
    
    KDTree * tree = malloc(sizeof(KDTree));
    if (!tree) {return NULL;}
    
    tree->size = n;
    
    /* copies points so array order can be adjusted for k-d tree 
    construction without affecting user's order */
    MetaPoint * stored_points = malloc(sizeof(MetaPoint) * n);
    if (!stored_points) {return NULL;}
    
    for (size_t i=0; i<n; i++) {
        stored_points[i].index = i;
        stored_points[i].data = points[i];
    }    
    kd_build_impl(tree, stored_points, n, 0);
    
    free(stored_points);
    return tree;
}

void kd_build_impl(KDTree * tree, MetaPoint * points, size_t n, unsigned int dim) {
    /* does not guard against NULL inputs because functions that call
    this one already handle that */
    if (n == 0) {return;}
    
    unsigned int median_i = n / 2;
    unsigned int next_dim = (dim+1) % 3;
    
    MetaPoint currPt = quick_select_k_meta_point(points, n, median_i, dim);
    kd_insert(tree, currPt.data, currPt.index);
    
    kd_build_impl(tree, points, median_i, next_dim);
    kd_build_impl(tree, points+median_i+1, n-median_i-1, next_dim);
}

size_t kd_cube_query_impl(
    KDNode * node, Point * cube_min_p, Point * cube_max_p,
    size_t min_index, size_t *indices, size_t * indices_i_p,
    unsigned int dim
);

/* operates in O(log(n) + k) time, where k is the number of retrieved keys */
size_t kd_cube_query(
    const KDTree *tree, 
    Point center, 
    double r,
    size_t min_index, 
    size_t *indices
) {
    if (!tree || !tree->root || !indices || min_index >= tree->size) {
        return 0;
    }

    // reduce memory use by building these once then using addresses
    Point cube_min = pt_build(center.x - r, center.y - r, center.z - r);
    Point * cube_min_p = &cube_min;

    Point cube_max = pt_build(center.x + r, center.y + r, center.z + r);
    Point * cube_max_p = &cube_max;

    /* keeps a record of where the pointer to indices lies so no 
       overwrites occur */
    size_t indices_i = 0;
    size_t * indices_i_p = &indices_i;
    
    return kd_cube_query_impl(
        tree->root, cube_min_p, cube_max_p, 
        min_index, indices, indices_i_p, 0
    );
}

size_t kd_cube_query_impl(
    KDNode * node, Point * cube_min_p, Point * cube_max_p,
    size_t min_index, size_t *indices, size_t * indices_i_p,
    unsigned int dim
) {
    if (!node || node->max_index < min_index) {return 0;}

    enum comp min_curr_comp = compare_points(
        *cube_min_p, node->data, dim
    );
    enum comp curr_max_comp = compare_points(
        node->data, *cube_max_p, dim
    );

    /* if point is outside query range, then, because of how duplicates are
    handled, there's always a need to check all the way down the subtrees 
    nearest to the edges of the query cube */
    unsigned int next_dim = (dim+1) % 3;
    if (min_curr_comp == GT) {
        return kd_cube_query_impl(
            node->right, cube_min_p, cube_max_p,
            min_index, indices, indices_i_p, next_dim
        );
    } else if (curr_max_comp == GT) {
        return kd_cube_query_impl(
            node->left, cube_min_p, cube_max_p,
            min_index, indices, indices_i_p, next_dim
        );
    }

    /* this endpoint means point falls in range and both subtrees need to be 
       searched */
    int point_eligible = (
        node->index >= min_index 
        && is_point_in_rect_prism(node->data, *cube_min_p, *cube_max_p)
    );
    if (point_eligible) {
        indices[*indices_i_p] = node->index;
        (*indices_i_p)++;
    }

    return point_eligible + kd_cube_query_impl(
        node->left, cube_min_p, cube_max_p, 
        min_index, indices, indices_i_p, next_dim
    ) + kd_cube_query_impl(
        node->right, cube_min_p, cube_max_p, 
        min_index, indices, indices_i_p, next_dim
    );
}

void kd_free_impl(KDNode *node);

void kd_free(KDTree *tree) {
    if (!tree) {return;}
    kd_free_impl(tree->root);
    free(tree);
}

void kd_free_impl(KDNode *node) {
    if (!node) {return;}
    kd_free_impl(node->left);
    kd_free_impl(node->right);
    free(node);
}

/* graph_stats is an AI assistance permitted function; AI assistance 
   is used for this function only */
GraphStats graph_stats(const Point *points, size_t n, double r) {
    GraphStats stats = {0, 1};

    /* zero or one point: no edges possible, trivially connected */
    if (!points || n <= 1) {
        return stats;
    }

    KDTree *tree = kd_build(points, n);
    size_t *indices = malloc(sizeof(size_t) * n);
    size_t *parent = malloc(sizeof(size_t) * n);

    if (!tree || !indices || !parent) {
        kd_free(tree);   /* kd_free handles NULL */
        free(indices);
        free(parent);
        stats.connected = 0;
        return stats;
    }

    /* union-find, used to determine connectivity as edges are found */
    for (size_t i = 0; i < n; i++) {
        parent[i] = i;
    }
    size_t components = n;
    double r_sq = r * r;

    for (size_t i = 0; i + 1 < n; i++) {
        /* min_index = i+1 means each pair is only considered once */
        size_t found = kd_cube_query(tree, points[i], r, i + 1, indices);

        for (size_t k = 0; k < found; k++) {
            size_t j = indices[k];

            /* the cube is a superset of the sphere, so verify with
               euclidean distance (squared, to avoid sqrt) */
            double dx = points[i].x - points[j].x;
            double dy = points[i].y - points[j].y;
            double dz = points[i].z - points[j].z;
            if (dx * dx + dy * dy + dz * dz > r_sq) {
                continue;
            }

            stats.edges++;

            /* find roots with path halving */
            size_t a = i;
            while (parent[a] != a) {
                parent[a] = parent[parent[a]];
                a = parent[a];
            }
            size_t b = j;
            while (parent[b] != b) {
                parent[b] = parent[parent[b]];
                b = parent[b];
            }

            if (a != b) {
                parent[a] = b;
                components--;
            }
        }
    }

    stats.connected = (components == 1);

    kd_free(tree);
    free(indices);
    free(parent);
    return stats;
}

// implementation functions
    
int kd_insert(KDTree * tree, Point pt, size_t index) {
    if (!tree) {return 0;}
    
    KDNode ** currNode_p = &tree->root;
    KDNode * currNode = *currNode_p;
    unsigned int dim = 0;
    enum comp comp_out;

    while (1) {
        if (!*currNode_p) {
            *currNode_p = kd_node_build(pt, index);
            if (*currNode_p) {return 1;} // success
            else {return 0;} // fail
        }
        
        if (index > currNode->max_index) {
            currNode->max_index = index;
        }
        
        comp_out = compare_points(pt, currNode->data, dim);
        
        dim = (dim+1) % 3;
        if (comp_out == LT) {
            currNode_p = &currNode->left;
            currNode = *currNode_p;
            continue;
        } else {
            currNode_p = &currNode->right;
            currNode = *currNode_p;
            continue;
        }
    }
}

KDNode * kd_node_build(Point pt, size_t index) {
    KDNode * out = malloc(sizeof(KDNode));
    if (!out) {return NULL;}

    out->data = pt;
    out->index = index;
    out->max_index = index;

    out->left = NULL;
    out->right = NULL;

    return out;
}

int is_point_in_rect_prism(Point pt, Point min_vert, Point max_vert) {
    return (
        compare_points(pt, min_vert, 0) != LT
        && compare_points(pt, min_vert, 1) != LT
        && compare_points(pt, min_vert, 2) != LT
        && compare_points(pt, max_vert, 0) != GT
        && compare_points(pt, max_vert, 1) != GT
        && compare_points(pt, max_vert, 2) != GT
    );
}

Point pt_build(double x, double y, double z) {
    Point pt;
    pt.x = x;
    pt.y = y;
    pt.z = z;
    return pt;
}

enum comp compare_points(Point pt1, Point pt2, unsigned int dim) {
    double v1, v2;

    switch (dim) {
        case 0: // x
        v1 = pt1.x;
        v2 = pt2.x;
        break;

        case 1: // y
        v1 = pt1.y;
        v2 = pt2.y;
        break;

        case 2: // z
        v1 = pt1.z;
        v2 = pt2.z;
        break;

        default:
        return FAIL; 
    }

    if (v1 < v2) { return LT; }
    if (v1 > v2) { return GT; }
    return EQ;
}

/* takes the last point of a section of an array and rearranges such 
   that, for that section, all points left of that element are less 
   than it and all points right are greater than it and returns its 
   new index

   note: does not verify points is not null, assumes caller will
   check 

   TODO:
   this uses lomuto's scheme, when optimizing I may switch to hoare's
   */
size_t partition_points(
    Point * points, 
    size_t first, 
    size_t last, 
    unsigned int dim
) {
    // selects based on first, middle, and last point's median
    set_partition_pivot_to_last(
        points, first, last, dim
    );
    Point pivot = points[last];
    size_t pivot_index = first;
    Point temp;

    for (size_t i=first; i<last; i++) {
        if (compare_points(points[i], pivot, dim) == LT) {
            if (i != pivot_index) {
                temp = points[i];
                points[i] = points[pivot_index];
                points[pivot_index] = temp;
            }
            pivot_index++;
        }
    }

    points[last] = points[pivot_index];
    points[pivot_index] = pivot;

    return pivot_index;
}

void set_partition_pivot_to_last(
    Point * points, 
    size_t first, 
    size_t last, 
    unsigned int dim
) {
    size_t middle = (last-first)/2;

    Point first_pt = points[first];
    Point middle_pt = points[middle];
    Point last_pt = points[last];

    enum comp fm_comp = compare_points(first_pt, middle_pt, dim);
    enum comp ml_comp = compare_points(middle_pt, last_pt, dim);
    enum comp lf_comp = compare_points(last_pt, first_pt, dim);

    /* values compared with equals because the median just needs to
       be less than one and greater than the other, or be equal 
       ex: (first < middle < last) -> signs same, middle is median */

    if (lf_comp == fm_comp || fm_comp == EQ) {
        points[first] = last_pt;
        points[last] = first_pt;
        return;
    }
    if (fm_comp == ml_comp) {
        points[middle] = last_pt;
        points[last] = middle_pt;
        return;
    }
}

/* finds k+1 th smallest element and modifies given array of Points
such that all points left of said element are less than that element
and all points right are greater than that element*/
Point quick_select_k_point(
    Point * points, size_t n, unsigned int k, unsigned int dim
) {
    size_t first = 0;
    size_t last = n-1;
    
    size_t curr_k = partition_points(points, first, last, dim);
    while (curr_k != k) {
        if (curr_k < k) {
            first = curr_k+1;
        } else {
            last = curr_k-1;
        }
        
        curr_k = partition_points(points, first, last, dim);
    }
    return points[k];
}

/*
The next several functions are copies of the above functions using the
MetaPoint struct. This is to allow the kd builder to see the original 
index of any given point, so the additional 
*/

/* takes the last point of a section of an array and rearranges such 
   that, for that section, all points left of that element are less 
   than it and all points right are greater than it and returns its 
   new index

   note: does not verify points is not null, assumes caller will
   check 

   TODO:
   this uses lomuto's scheme, when optimizing I may switch to hoare's
   */
size_t partition_meta_points(
    MetaPoint * points, 
    size_t first, 
    size_t last, 
    unsigned int dim
) {
    // selects based on first, middle, and last point's median
    meta_set_partition_pivot_to_last(
        points, first, last, dim
    );
    MetaPoint pivot = points[last];
    size_t pivot_index = first;
    MetaPoint temp;

    for (size_t i=first; i<last; i++) {
        if (compare_points(points[i].data, pivot.data, dim) == LT) {
            if (i != pivot_index) {
                temp = points[i];
                points[i] = points[pivot_index];
                points[pivot_index] = temp;
            }
            pivot_index++;
        }
    }

    points[last] = points[pivot_index];
    points[pivot_index] = pivot;

    return pivot_index;
}

void meta_set_partition_pivot_to_last(
    MetaPoint * points, 
    size_t first, 
    size_t last, 
    unsigned int dim
) {
    size_t middle = (last-first)/2;

    MetaPoint first_pt = points[first];
    MetaPoint middle_pt = points[middle];
    MetaPoint last_pt = points[last];

    enum comp fm_comp = compare_points(
        first_pt.data, middle_pt.data, dim
    );
    enum comp ml_comp = compare_points(
        middle_pt.data, last_pt.data, dim
    );
    enum comp lf_comp = compare_points(
        last_pt.data, first_pt.data, dim
    );

    /* values compared with equals because the median just needs to
       be less than one and greater than the other, or be equal 
       ex: (first < middle < last) -> signs same, middle is median */

    if (lf_comp == fm_comp || fm_comp == EQ) {
        points[first] = last_pt;
        points[last] = first_pt;
        return;
    }
    if (fm_comp == ml_comp) {
        points[middle] = last_pt;
        points[last] = middle_pt;
        return;
    }
}

/* copy of quick_select_k_point, except with the MetaPoint struct this
   is meant for the build process, which requires the builder to know 
   what index each point was originally stored at*/
MetaPoint quick_select_k_meta_point(
    MetaPoint * points, size_t n, unsigned int k, unsigned int dim
) {
    size_t first = 0;
    size_t last = n-1;
    
    size_t curr_k = partition_meta_points(points, first, last, dim);
    while (curr_k != k) {
        if (curr_k < k) {
            first = curr_k+1;
        } else {
            last = curr_k-1;
        }
        
        curr_k = partition_meta_points(points, first, last, dim);
    }
    return points[k];
}