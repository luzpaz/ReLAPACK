#include "test.h"

datatype *A[2];

void pre() {
    x2matgen(n, n, A[0], A[1]);
}

void post() {
    error = x2vecerr(n * n, A[0], A[1]);
}

void tests() {
    A[0] = xmalloc(n * n);
    A[1] = xmalloc(n * n);

    #define ROUTINE XPREF(trtri)

    TEST("L", "N", &n, A[i], &n, &info);
    TEST("U", "N", &n, A[i], &n, &info);

    free(A[0]);
    free(A[1]);
}
