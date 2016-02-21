#include "relapack.h"
#if XSYTRF_ALLOW_MALLOC
#include <stdlib.h>
#endif

static void RELAPACK_chetrf_rook_rec(const char *, const int *, const int *, int *,
    float *, const int *, int *, float *, const int *, int *);


/** CHETRF_ROOK computes the factorization of a complex Hermitian indefinite matrix using the bounded Bunch-Kaufman ("rook") diagonal pivoting method.
 *
 * This routine is functionally equivalent to LAPACK's chetrf_rook.
 * For details on its interface, see
 * http://www.netlib.org/lapack/explore-html/d0/d5e/chetrf__rook_8f.html
 * */
void RELAPACK_chetrf_rook(
    const char *uplo, const int *n,
    float *A, const int *ldA, int *ipiv,
    float *Work, const int *lWork, int *info
) {

    // Required work size
    const int cleanlWork = *n * (*n / 2);
    int minlWork = cleanlWork;
#if XSYTRF_ALLOW_MALLOC
    minlWork = 1;
#endif

    // Check arguments
    const int lower = LAPACK(lsame)(uplo, "L");
    const int upper = LAPACK(lsame)(uplo, "U");
    *info = 0;
    if (!lower && !upper)
        *info = -1;
    else if (*n < 0)
        *info = -2;
    else if (*ldA < MAX(1, *n))
        *info = -4;
    else if (*lWork < minlWork && *lWork != -1)
        *info = -7;
    if (*info) {
        const int minfo = -*info;
        LAPACK(xerbla)("CHETRF", &minfo);
        return;
    }

    if (*lWork == -1) {
        // Work size query
        *Work = cleanlWork;
        return;
    }

    // Clean char * arguments
    const char cleanuplo = lower ? 'L' : 'U';

    // Ensure Work size
    float *cleanWork = Work;
#if XSYTRF_ALLOW_MALLOC
    if (*lWork < cleanlWork)
        cleanWork = malloc(cleanlWork * 2 * sizeof(float));
#endif

    int nout;
    RELAPACK_chetrf_rook_rec(&cleanuplo, n, n, &nout, A, ldA, ipiv, cleanWork, n, info);

#if XSYTRF_ALLOW_MALLOC
    if (cleanWork != Work)
        free(cleanWork);
#endif
}

/** chetrf_rook's recursive compute kernel */
static void RELAPACK_chetrf_rook_rec(
    const char *uplo, const int *n_full, const int *n, int *n_out,
    float *A, const int *ldA, int *ipiv,
    float *Work, const int *ldWork, int *info
) {

    // top recursion level?
    const int top = *n_full == *n;

    if (*n <= MAX(CROSSOVER_CHETRF, 3)) {
        // Unblocked
        if (top) {
            LAPACK(chetf2)(uplo, n, A, ldA, ipiv, info);
            *n_out = *n;
        } else
            FORTRAN(relapack_chetrf_rook_rec2)(uplo, n_full, n, n_out, A, ldA, ipiv, Work, ldWork, info);
        return;
    }

    int info1, info2;

    // Constants
    const float ONE[]  = {1, 0};
    const float MONE[] = {-1, 0};
    const int  iONE[]  = {1};

    const int n_rest = *n_full - *n;

    if (*uplo == 'L') {
        // Splitting (setup)
        int n1 = REC_SPLIT(*n);
        int n2 = *n - n1;

        // Work_L *
        float *const Work_L = Work;

        // recursion(A_L)
        int n1_out;
        RELAPACK_chetrf_rook_rec(uplo, n_full, &n1, &n1_out, A, ldA, ipiv, Work_L, ldWork, &info1);
        n1 = n1_out;

        // Splitting (continued)
        n2 = *n - n1;
        const int n_full2 = *n_full - n1;

        // *      *
        // A_BL   A_BR
        // A_BL_B A_BR_B
        float *const A_BL   = A                 + 2 * n1;
        float *const A_BR   = A + 2 * *ldA * n1 + 2 * n1;
        float *const A_BL_B = A                 + 2 * *n;
        float *const A_BR_B = A + 2 * *ldA * n1 + 2 * *n;

        // *        *
        // Work_BL Work_BR
        // *       *
        // (top recursion level: use Work as Work_BR)
        const int ldWork_BR  = top ? n2   : *ldWork;
        float *const Work_BL =              Work                    + 2 * n1;
        float *const Work_BR = top ? Work : Work + 2 * *ldWork * n1 + 2 * n1;

        // ipiv_T
        // ipiv_B
        int *const ipiv_B = ipiv + n1;

        // A_BR = A_BR - A_BL Work_BL'
        RELAPACK_cgemm_tr_rec("N", "T", uplo, &n2, &n1, MONE, A_BL, ldA, Work_BL, ldWork, ONE, A_BR, ldA);
        BLAS(cgemm)("N", "T", &n_rest, &n2, &n1, MONE, A_BL_B, ldA, Work_BL, ldWork, ONE, A_BR_B, ldA);

        // recursion(A_BR)
        int n2_out;
        RELAPACK_chetrf_rook_rec(uplo, &n_full2, &n2, &n2_out, A_BR, ldA, ipiv_B, Work_BR, &ldWork_BR, &info2);

        if (n2_out != n2) {
            // undo 1 column of updates
            const int n_restp1 = n_rest + 1;

            // last column of A_BR
            float *const A_BR_r = A_BR + 2 * *ldA * n2_out + 2 * n2_out;

            // last row of A_BL
            float *const A_BL_b = A_BL + 2 * n2_out;

            // last row of Work_BL
            float *const Work_BL_b = Work_BL + 2 * n2_out;

            // A_BR_r = A_BR_r + A_BL_b Work_BL_b'
            BLAS(cgemv)("N", &n_restp1, &n1, ONE, A_BL_b, ldA, Work_BL_b, ldWork, ONE, A_BR_r, iONE);
        }
        n2 = n2_out;

        // shift pivots
        int i;
        for (i = 0; i < n2; i++)
            if (ipiv_B[i] > 0)
                ipiv_B[i] += n1;
            else
                ipiv_B[i] -= n1;

        *info = info1 || info2;
        *n_out = n1 + n2;
    } else {
        // Splitting (setup)
        int n2 = REC_SPLIT(*n);
        int n1 = *n - n2;

        // * Work_R
        // (top recursion level: use Work as Work_R)
        float *const Work_R = top ? Work : Work + 2 * *ldWork * n1;

        // recursion(A_R)
        int n2_out;
        RELAPACK_chetrf_rook_rec(uplo, n_full, &n2, &n2_out, A, ldA, ipiv, Work_R, ldWork, &info2);
        const int n2_diff = n2 - n2_out;
        n2 = n2_out;

        // Splitting (continued)
        n1 = *n - n2;
        const int n_full1 = *n_full - n2;

        // * A_TL_T A_TR_T
        // * A_TL   A_TR
        // * *      *
        float *const A_TL_T = A + 2 * *ldA * n_rest;
        float *const A_TR_T = A + 2 * *ldA * (n_rest + n1);
        float *const A_TL   = A + 2 * *ldA * n_rest        + 2 * n_rest;
        float *const A_TR   = A + 2 * *ldA * (n_rest + n1) + 2 * n_rest;

        // Work_L *
        // *      Work_TR
        // *      *
        // (top recursion level: Work_R was Work)
        const int ldWork_L   = top ? n1 : *ldWork;
        float *const Work_L  = Work;
        float *const Work_TR = Work + 2 * *ldWork * (top ? n2_diff : n1) + 2 * n_rest;

        // A_TL = A_TL - A_TR Work_TR'
        RELAPACK_cgemm_tr_rec("N", "T", uplo, &n1, &n2, MONE, A_TR, ldA, Work_TR, ldWork, ONE, A_TL, ldA);
        BLAS(cgemm)("N", "T", &n_rest, &n1, &n2, MONE, A_TR_T, ldA, Work_TR, ldWork, ONE, A_TL_T, ldA);

        // recursion(A_TL)
        int n1_out;
        RELAPACK_chetrf_rook_rec(uplo, &n_full1, &n1, &n1_out, A, ldA, ipiv, Work_L, &ldWork_L, &info1);

        if (n1_out != n1) {
            // undo 1 column of updates
            const int n_restp1 = n_rest + 1;

            // A_TL_T_l = A_TL_T_l + A_TR_T Work_TR_t'
            BLAS(cgemv)("N", &n_restp1, &n2, ONE, A_TR_T, ldA, Work_TR, ldWork, ONE, A_TL_T, iONE);
        }
        n1 = n1_out;

        *info  = info2 || info1;
        *n_out = n1 + n2;
    }
}
