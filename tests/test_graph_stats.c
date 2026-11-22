#include "test_graph_stats.h"
#include "../a4.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------ */
/* bookkeeping                                                         */
/* ------------------------------------------------------------------ */

static int gs_run;
static int gs_failed;

static void gs_pass(const char *name, const char *why) {
    gs_run++;
    printf("  [PASS] %s\n         %s\n", name, why);
}

static void gs_fail(const char *name, const char *why, const char *detail) {
    gs_run++;
    gs_failed++;
    printf("  [FAIL] %s\n         expected behavior: %s\n         %s\n",
           name, why, detail);
}

/* Runs graph_stats and compares against the expected values. */
static void gs_check(const char *name, const char *why, const Point *pts,
                     size_t n, double r, size_t exp_edges, int exp_conn) {
    GraphStats g = graph_stats(pts, n, r);
    if (g.edges == exp_edges && g.connected == exp_conn) {
        gs_pass(name, why);
    } else {
        char buf[200];
        snprintf(buf, sizeof buf,
                 "expected edges=%zu connected=%d, got edges=%zu connected=%d",
                 exp_edges, exp_conn, g.edges, g.connected);
        gs_fail(name, why, buf);
    }
}

/* ------------------------------------------------------------------ */
/* brute-force reference implementation (O(n^2), independent of k-d)   */
/* ------------------------------------------------------------------ */

static size_t ref_find(size_t *p, size_t x) {
    while (p[x] != x) {
        p[x] = p[p[x]];
        x = p[x];
    }
    return x;
}

static GraphStats ref_stats(const Point *pts, size_t n, double r) {
    GraphStats g = {0, 1};
    if (n <= 1) return g;

    size_t *p = malloc(sizeof(size_t) * n);
    if (!p) { g.connected = -1; return g; }
    for (size_t i = 0; i < n; i++) p[i] = i;

    size_t comps = n;
    double r_sq = r * r;
    for (size_t i = 0; i < n; i++) {
        for (size_t j = i + 1; j < n; j++) {
            double dx = pts[i].x - pts[j].x;
            double dy = pts[i].y - pts[j].y;
            double dz = pts[i].z - pts[j].z;
            if (dx * dx + dy * dy + dz * dz <= r_sq) {
                g.edges++;
                size_t a = ref_find(p, i), b = ref_find(p, j);
                if (a != b) { p[a] = b; comps--; }
            }
        }
    }
    g.connected = (comps == 1);
    free(p);
    return g;
}

/* Compares graph_stats to the brute-force reference. */
static void gs_check_vs_ref(const char *name, const char *why,
                            const Point *pts, size_t n, double r) {
    GraphStats ref = ref_stats(pts, n, r);
    GraphStats got = graph_stats(pts, n, r);
    if (ref.edges == got.edges && ref.connected == got.connected) {
        gs_pass(name, why);
    } else {
        char buf[200];
        snprintf(buf, sizeof buf,
                 "reference edges=%zu connected=%d, got edges=%zu connected=%d",
                 ref.edges, ref.connected, got.edges, got.connected);
        gs_fail(name, why, buf);
    }
}

/* ------------------------------------------------------------------ */
/* deterministic RNG so failures are reproducible                      */
/* ------------------------------------------------------------------ */

static uint64_t gs_rng_state = 1;

static void gs_seed(uint64_t s) { gs_rng_state = s ? s : 1; }

static double gs_rand01(void) {
    gs_rng_state = gs_rng_state * 6364136223846793005ULL
                   + 1442695040888963407ULL;
    return (double)(gs_rng_state >> 11) / 9007199254740992.0; /* [0,1) */
}

static Point *gs_random_points(size_t n, double lo, double hi) {
    Point *p = malloc(sizeof(Point) * n);
    if (!p) return NULL;
    for (size_t i = 0; i < n; i++) {
        p[i].x = lo + (hi - lo) * gs_rand01();
        p[i].y = lo + (hi - lo) * gs_rand01();
        p[i].z = lo + (hi - lo) * gs_rand01();
    }
    return p;
}

/* ------------------------------------------------------------------ */
/* test groups                                                         */
/* ------------------------------------------------------------------ */

static void test_trivial_sizes(void) {
    printf("\n-- trivial sizes --\n");
    Point one[] = {{1.0, 2.0, 3.0}};

    gs_check("n=1 reports 0 edges, connected=1",
             "spec: a single point has no edges and is connected",
             one, 1, 1.0, 0, 1);
    gs_check("n=1 with r=0",
             "single point stays connected regardless of r",
             one, 1, 0.0, 0, 1);
    gs_check("n=1 with huge r",
             "single point never has an edge to itself",
             one, 1, 1e9, 0, 1);

    /* NOT in the spec: these are the behaviors I chose; change them here
       if your assignment expects otherwise */
    gs_check("n=0 (assumed behavior: 0 edges, connected=1)",
             "n=0 is unspecified; implementation treats it like n=1",
             one, 0, 1.0, 0, 1);
    gs_check("NULL points (assumed behavior: 0 edges, connected=1)",
             "NULL input must not crash",
             NULL, 5, 1.0, 0, 1);
}

static void test_two_points(void) {
    printf("\n-- two points --\n");
    Point close[] = {{0, 0, 0}, {1, 0, 0}};
    Point far[]   = {{0, 0, 0}, {10, 0, 0}};

    gs_check("two points within r form one edge",
             "distance 1 <= r 2: one edge, connected",
             close, 2, 2.0, 1, 1);
    gs_check("two points beyond r form no edge",
             "distance 10 > r 2: no edge, not connected",
             far, 2, 2.0, 0, 0);

    gs_check("two points, query order independent",
             "swapping point order must not change the result",
             (Point[]){{1, 0, 0}, {0, 0, 0}}, 2, 2.0, 1, 1);
}

static void test_boundaries(void) {
    printf("\n-- distance boundary behavior --\n");
    /* 3-4-5 triangle: squared distance 25 is exact in floating point */
    Point pyth[] = {{0, 0, 0}, {3, 4, 0}};
    gs_check("distance exactly r counts as an edge",
             "spec: distance <= r is an edge (3-4-5, r=5)",
             pyth, 2, 5.0, 1, 1);
    gs_check("distance slightly more than r is not an edge",
             "r=4.999 < 5: no edge",
             pyth, 2, 4.999, 0, 0);

    /* inside the query cube but outside the sphere */
    Point corner[] = {{0, 0, 0}, {1, 1, 1}};
    gs_check("cube corner outside sphere is rejected",
             "point inside the cube but at distance sqrt(3) > r=1 "
             "must not be an edge (Euclidean verification)",
             corner, 2, 1.0, 0, 0);
    gs_check("cube corner accepted when r large enough",
             "sqrt(3) ~ 1.732 <= r=1.75: edge exists",
             corner, 2, 1.75, 1, 1);

    Point diag2[] = {{0, 0, 0}, {1, 1, 0}};
    gs_check("2D diagonal outside sphere is rejected",
             "distance sqrt(2) ~ 1.414 > r=1.4: no edge",
             diag2, 2, 1.4, 0, 0);
    gs_check("2D diagonal inside sphere is accepted",
             "distance sqrt(2) ~ 1.414 <= r=1.5: edge",
             diag2, 2, 1.5, 1, 1);
}

static void test_zero_radius_and_duplicates(void) {
    printf("\n-- zero radius and duplicate points --\n");
    Point distinct[] = {{0, 0, 0}, {1, 0, 0}, {2, 0, 0}};
    gs_check("r=0 with distinct points: no edges",
             "no two distinct points are within distance 0",
             distinct, 3, 0.0, 0, 0);

    Point twin[] = {{4, 4, 4}, {4, 4, 4}};
    gs_check("r=0 with two identical points: one edge",
             "distance 0 <= r 0, so duplicates are connected",
             twin, 2, 0.0, 1, 1);

    Point triple[] = {{1, 2, 3}, {1, 2, 3}, {1, 2, 3}};
    gs_check("three identical points form a triangle",
             "3 choose 2 = 3 edges, each pair counted once",
             triple, 3, 1.0, 3, 1);

    Point mixed[] = {{0, 0, 0}, {0, 0, 0}, {100, 100, 100}};
    gs_check("duplicates plus a far outlier",
             "one edge between the duplicates, outlier isolated",
             mixed, 3, 1.0, 1, 0);

    /* 5 locations 10 apart, 10 copies of each */
    Point dup[50];
    for (int g = 0; g < 5; g++)
        for (int k = 0; k < 10; k++)
            dup[g * 10 + k] = (Point){10.0 * g, 0, 0};
    gs_check("5 groups of 10 duplicates, r=1",
             "each group is a clique: 5 * C(10,2) = 225 edges, "
             "groups disconnected",
             dup, 50, 1.0, 225, 0);
    gs_check("5 groups of 10 duplicates, r huge",
             "everything within r: C(50,2) = 1225 edges, connected",
             dup, 50, 1000.0, 1225, 1);
    gs_check("5 groups of 10 duplicates, r bridges groups",
             "r=10 links neighbouring groups: 225 + 4*100 = 625 edges, "
             "connected",
             dup, 50, 10.0, 625, 1);
}

static void test_structures(void) {
    printf("\n-- known graph shapes --\n");

    Point chain[] = {{0, 0, 0}, {1, 0, 0}, {2, 0, 0}, {3, 0, 0}, {4, 0, 0}};
    gs_check("chain of 5, r=1",
             "spacing 1 <= r: 4 edges, connected (path graph)",
             chain, 5, 1.0, 4, 1);
    gs_check("chain of 5, r=0.5",
             "spacing 1 > r: no edges, not connected",
             chain, 5, 0.5, 0, 0);
    gs_check("chain of 5, r=2",
             "pairs within 2: 4 + 3 = 7 edges, connected",
             chain, 5, 2.0, 7, 1);
    gs_check("chain of 5, r=4 (complete)",
             "all pairs within 4: C(5,2) = 10 edges",
             chain, 5, 4.0, 10, 1);

    Point chain_rev[] = {{4, 0, 0}, {0, 0, 0}, {2, 0, 0}, {3, 0, 0}, {1, 0, 0}};
    gs_check("shuffled chain of 5, r=1",
             "input order must not matter: still 4 edges, connected",
             chain_rev, 5, 1.0, 4, 1);

    Point two_tri[] = {
        {0, 0, 0}, {1, 0, 0}, {0, 1, 0},
        {100, 0, 0}, {101, 0, 0}, {100, 1, 0}
    };
    gs_check("two separate triangles",
             "3 + 3 = 6 edges, two components so connected=0",
             two_tri, 6, 2.0, 6, 0);

    Point clique_iso[] = {
        {0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {50, 50, 50}
    };
    gs_check("clique of 4 plus an isolated point",
             "C(4,2)=6 edges; the isolated point makes connected=0",
             clique_iso, 5, 2.0, 6, 0);

    Point star[] = {
        {0, 0, 0}, {5, 0, 0}, {-5, 0, 0}, {0, 5, 0}, {0, -5, 0}
    };
    gs_check("star: hub with 4 leaves, r=5",
             "hub-to-leaf edges (4); leaf-to-leaf distance 7.07 > 5. "
             "Connected through the hub",
             star, 5, 5.0, 4, 1);

    Point diag_chain[] = {{0, 0, 0}, {1, 1, 1}, {2, 2, 2}, {3, 3, 3}};
    gs_check("diagonal chain, r=1.75",
             "neighbours are sqrt(3)=1.73 apart so 3 edges; next-nearest "
             "are 3.46 away. Cube would also catch these diagonals",
             diag_chain, 4, 1.75, 3, 1);
    gs_check("diagonal chain, r=1.7",
             "sqrt(3)=1.732 > 1.7: no edges despite cube overlap",
             diag_chain, 4, 1.7, 0, 0);

    Point neg[] = {{-5, -5, -5}, {-4, -5, -5}, {-3, -5, -5}};
    gs_check("negative coordinates",
             "chain along x at negative coordinates: 2 edges, connected",
             neg, 3, 1.0, 2, 1);

    Point straddle[] = {{-0.5, 0, 0}, {0.5, 0, 0}};
    gs_check("points straddling the origin",
             "distance 1 <= r 1: edge across zero",
             straddle, 2, 1.0, 1, 1);
}

static void test_axis_degenerate(void) {
    printf("\n-- collinear / planar degenerate inputs --\n");
    /* all points share x and y, vary in z only */
    Point line[100];
    for (int i = 0; i < 100; i++) line[i] = (Point){3, 7, (double)i};

    gs_check("100 points on the z axis, r=1",
             "99 edges between neighbours, connected",
             line, 100, 1.0, 99, 1);
    gs_check("100 points on the z axis, r=2",
             "99 + 98 = 197 edges, connected",
             line, 100, 2.0, 197, 1);
    gs_check("100 points on the z axis, r=0.9",
             "spacing 1 > 0.9: no edges, not connected",
             line, 100, 0.9, 0, 0);

    /* all points share z (planar 10x10 grid) */
    Point plane[100];
    for (int i = 0; i < 10; i++)
        for (int j = 0; j < 10; j++)
            plane[i * 10 + j] = (Point){(double)i, (double)j, 0};
    gs_check("10x10 planar grid, r=1",
             "axis neighbours only: 2*10*9 = 180 edges, connected",
             plane, 100, 1.0, 180, 1);
    gs_check("10x10 planar grid, r=1.5",
             "adds diagonals (sqrt2 <= 1.5): 180 + 2*9*9 = 342 edges",
             plane, 100, 1.5, 342, 1);
}

static void test_grid(void) {
    printf("\n-- 3D integer grid (many equal coordinates) --\n");
    enum { G = 6 };
    Point grid[G * G * G];
    size_t n = 0;
    for (int x = 0; x < G; x++)
        for (int y = 0; y < G; y++)
            for (int z = 0; z < G; z++)
                grid[n++] = (Point){(double)x, (double)y, (double)z};

    gs_check("6x6x6 grid, r=0.5",
             "grid spacing 1 > r: no edges, not connected",
             grid, n, 0.5, 0, 0);
    gs_check("6x6x6 grid, r=1",
             "axis neighbours only: 3 * 5*6*6 = 540 edges, connected",
             grid, n, 1.0, 540, 1);
    gs_check("6x6x6 grid, r=1.5",
             "adds face diagonals (sqrt2): 540 + 3*2*5*5*6 = 1440 edges",
             grid, n, 1.5, 1440, 1);
    gs_check("6x6x6 grid, r=1.75",
             "adds body diagonals (sqrt3): 1440 + 4*5*5*5 = 1940 edges",
             grid, n, 1.75, 1940, 1);
    gs_check("6x6x6 grid, r huge",
             "complete graph: C(216,2) = 23220 edges",
             grid, n, 1000.0, 23220, 1);
}

static void test_random_vs_reference(void) {
    printf("\n-- random inputs vs brute-force reference --\n");
    static const double radii[] = {0.0, 0.3, 0.8, 1.5, 2.5, 4.0, 30.0};
    static const size_t sizes[] = {2, 3, 7, 50, 200};
    char name[120];

    uint64_t seed = 12345;
    for (size_t si = 0; si < sizeof sizes / sizeof *sizes; si++) {
        for (size_t ri = 0; ri < sizeof radii / sizeof *radii; ri++) {
            gs_seed(seed++);
            Point *p = gs_random_points(sizes[si], 0.0, 10.0);
            if (!p) { gs_fail("random alloc", "test setup", "malloc failed"); continue; }
            snprintf(name, sizeof name, "random n=%zu r=%.1f", sizes[si], radii[ri]);
            gs_check_vs_ref(name,
                            "edge count and connectivity match brute force",
                            p, sizes[si], radii[ri]);
            free(p);
        }
    }

    /* coordinate ranges that include negatives and larger magnitudes */
    gs_seed(777);
    Point *p = gs_random_points(300, -50.0, 50.0);
    if (p) {
        gs_check_vs_ref("random n=300 in [-50,50], r=15",
                        "negative coordinates, mid density", p, 300, 15.0);
        gs_check_vs_ref("random n=300 in [-50,50], r=40",
                        "dense graph", p, 300, 40.0);
        free(p);
    }

    /* large-ish stress */
    gs_seed(2024);
    p = gs_random_points(2000, 0.0, 10.0);
    if (p) {
        gs_check_vs_ref("random n=2000, r=0.5",
                        "sparse graph at scale", p, 2000, 0.5);
        gs_check_vs_ref("random n=2000, r=1.0",
                        "medium density at scale", p, 2000, 1.0);
        free(p);
    }
}

static void test_robustness(void) {
    printf("\n-- input handling / repeatability --\n");

    /* input array must be left untouched (const Point *) */
    Point orig[] = {{3, 1, 2}, {1, 5, 2}, {9, 9, 9}, {2, 2, 2}, {0, 0, 0}};
    Point copy[5];
    memcpy(copy, orig, sizeof orig);
    graph_stats(orig, 5, 4.0);
    if (memcmp(orig, copy, sizeof orig) == 0)
        gs_pass("input array is not modified",
                "graph_stats must not reorder or change the caller's points");
    else
        gs_fail("input array is not modified",
                "graph_stats must not reorder or change the caller's points",
                "caller's array differs after the call");

    /* same call twice should give identical results */
    GraphStats a = graph_stats(orig, 5, 4.0);
    GraphStats b = graph_stats(orig, 5, 4.0);
    if (a.edges == b.edges && a.connected == b.connected)
        gs_pass("repeated calls are deterministic",
                "no leftover static state between calls");
    else
        gs_fail("repeated calls are deterministic",
                "no leftover static state between calls",
                "two identical calls returned different results");

    /* many calls: catches gross leaks under ASan/valgrind, and state bugs */
    int ok = 1;
    for (int i = 0; i < 500; i++) {
        GraphStats g = graph_stats(orig, 5, 4.0);
        if (g.edges != a.edges || g.connected != a.connected) { ok = 0; break; }
    }
    if (ok)
        gs_pass("500 consecutive calls agree",
                "run under ASan/valgrind to confirm everything is freed");
    else
        gs_fail("500 consecutive calls agree",
                "no state carried between calls",
                "a later call returned a different result");
}

/* ------------------------------------------------------------------ */
/* entry point for your combined test runner                           */
/* ------------------------------------------------------------------ */

int run_graph_stats_tests(void) {
    gs_run = 0;
    gs_failed = 0;

    printf("==== graph_stats tests ====\n");
    test_trivial_sizes();
    test_two_points();
    test_boundaries();
    test_zero_radius_and_duplicates();
    test_structures();
    test_axis_degenerate();
    test_grid();
    test_random_vs_reference();
    test_robustness();

    printf("\n==== graph_stats: %d run, %d passed, %d failed ====\n",
           gs_run, gs_run - gs_failed, gs_failed);
    return gs_failed;
}