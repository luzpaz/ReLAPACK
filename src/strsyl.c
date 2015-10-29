#include "larpack.h"

void LARPACK(strsyl)(const char *tranA, const char *tranB, const int *isgn,
        const int *m, const int *n,
        const float *A, const int *ldA, const float *B, const int *ldB,
        float *C, const int *ldC, float *scale, int *info) {

    // Check arguments
    int notransA = LAPACK(lsame)(tranA, "N");
    int transA = LAPACK(lsame)(tranA, "T") || LAPACK(lsame)(tranA, "C");
    int notransB = LAPACK(lsame)(tranB, "N");
    int transB = LAPACK(lsame)(tranB, "T") || LAPACK(lsame)(tranB, "C");
    *info = 0;
    if (!transA && !notransA)
        *info = -1;
    else if (!transB && !notransB)
        *info = -2;
    else if (*isgn != 1 && *isgn != -1)
        *info = -3;
    else if (*m < 0)
        *info = -4;
    else if (*n < 0)
        *info = -5;
    else if (*ldA < MAX(1, *m))
        *info = -7;
    else if (*ldB < MAX(1, *n))
        *info = -9;
    else if (*ldC < MAX(1, *m))
        *info = -11;
    if (*info) {
        const int minfo = -*info;
        LAPACK(xerbla)("STRSYL", &minfo);
        return;
    }

    // Determine recursive splitting
    int splitm = 0, splitn = 0, m1, n1;

    // split m?
    if (*m > LARPACK_CROSSOVER) {
        // try ideal split
        m1 = (*m >= 16) ? ((*m + 8) / 16) * 8 : *m / 2;
        if (A[m1 + *ldA * (m1 - 1)] == 0)
            splitm = 1;
        else {
            // try to find a split close to m / 2
            int offset = 0;
            while (offset < *m / 2) {
                m1 = *m / 2 - offset; // left
                if (m1 > 0 &&  A[m1 + *ldA * (m1 - 1)] == 0) {
                    splitm = 1;
                    break;
                }
                m1 = *m / 2 + offset; // right
                if (m1 < *m && A[m1 + *ldA * (m1 - 1)] == 0) {
                    splitm = 1;
                    break;
                }
                offset += 1;
            }
        }
    } 

    // split n?
    if (*n > LARPACK_CROSSOVER) {
        // try ideal split
        n1 = (*n >= 16) ? ((*n + 8) / 16) * 8 : *n / 2;
        if (B[n1 + *ldB * (n1 - 1)] == 0)
            splitn = 1;
        else {
            // try to find a split close to n / 2
            int offset = 0;
            while (offset < *n / 2) {
                n1 = *n / 2 - offset; // left
                if (n1 > 0 && B[n1 + *ldB * (n1 - 1)] == 0) {
                    splitn = 1;
                    break;
                }
                n1 = *n / 2 + offset; // right
                if (n1 < *n && B[n1 + *ldB * (n1 - 1)] == 0) {
                    splitn = 1;
                    break;
                }
                offset += 1;
            }
        }
    } 

    if (!splitm && !splitn) {
        // Unblocked
        LAPACK(strsy2)(tranA, tranB, isgn, m, n, A, ldA, B, ldB, C, ldC, scale, info);
        return;
    }

    // Recursive

    // 1, -1, -isgn
   	const float s1[] = {1}, sm1[] = {-1}, smisgn[] = {-*isgn};
    // 0
    const int i0[] = {0};

    float scale1[] = {1}, scale2[] = {1};
    int info1[] = {0}, info2[] = {0};

    if (splitm && (!splitn || *m > *n)) {
        const int m2 = *m - m1;

        // A_TL A_TR
        //      A_BR
        const float *const A_TL = A;
        const float *const A_TR = A + *ldA * m1;
        const float *const A_BR = A + *ldA * m1 + m1;

        // C_T
        // C_B
        float *const C_T = C;
        float *const C_B = C + m1;

        if (notransA) {
            // recusion(A_BR, B, C_B)
            LARPACK(strsyl)(tranA, tranB, isgn, &m2, n, A_BR, ldA, B, ldB, C_B, ldC, scale1, info1);
            // C_T = C_T - A_TR * C_B
            BLAS(sgemm)("N", "N", &m1, n, &m2, sm1, A_TR, ldA, C_B, ldC, scale1, C_T, ldC);
            // recusion(A_TL, B, C_T)
            LARPACK(strsyl)(tranA, tranB, isgn, &m1, n, A_TL, ldA, B, ldB, C_T, ldC, scale2, info2);
            // apply scale
            if (scale2[0] != 1)
                LAPACK(slascl)("G", i0, i0, s1, scale2, &m2, n, C_B, ldC, info);
        } else {
            // recusion(A_TL, B, C_T)
            LARPACK(strsyl)(tranA, tranB, isgn, &m1, n, A_TL, ldA, B, ldB, C_T, ldC, scale1, info1);
            // C_B = C_B - A_TR' * C_T
            BLAS(sgemm)("C", "N", &m2, n, &m1, sm1, A_TR, ldA, C_T, ldC, scale1, C_B, ldC);
            // recusion(A_BR, B, C_B)
            LARPACK(strsyl)(tranA, tranB, isgn, &m2, n, A_BR, ldA, B, ldB, C_B, ldC, scale2, info2);
            // apply scale
            if (scale2[0] != 1)
                LAPACK(slascl)("G", i0, i0, s1, scale2, &m1, n, C_B, ldC, info);
        }
    } else {
        const int n2 = *n - n1;

        // B_TL B_TR
        //      B_BR
        const float *const B_TL = B;
        const float *const B_TR = B + *ldB * n1;
        const float *const B_BR = B + *ldB * n1 + n1;

        // C_L C_R
        float *const C_L = C;
        float *const C_R = C + *ldC * n1;

        if (notransB) {
            // recusion(A, B_TL, C_L)
            LARPACK(strsyl)(tranA, tranB, isgn, m, &n1, A, ldA, B_TL, ldB, C_L, ldC, scale1, info1);
            // C_R = C_R -/+ C_L * B_TR
            BLAS(sgemm)("N", "N", m, &n2, &n1, smisgn, C_L, ldC, B_TR, ldB, scale1, C_R, ldC);
            // recusion(A, B_BR, C_R)
            LARPACK(strsyl)(tranA, tranB, isgn, m, &n2, A, ldA, B_BR, ldB, C_R, ldC, scale2, info2);
            // apply scale
            if (scale2[0] != 1)
                LAPACK(slascl)("G", i0, i0, s1, scale2, m, &n1, C_L, ldC, info);
        } else {
            // recusion(A, B_BR, C_R)
            LARPACK(strsyl)(tranA, tranB, isgn, m, &n2, A, ldA, B_BR, ldB, C_R, ldC, scale1, info1);
            // C_L = C_L -/+ C_R * B_TR'
            BLAS(sgemm)("N", "C", m, &n1, &n2, smisgn, C_R, ldC, B_TR, ldB, scale1, C_L, ldC);
            // recusion(A, B_TL, C_L)
            LARPACK(strsyl)(tranA, tranB, isgn, m, &n1, A, ldA, B_TL, ldB, C_L, ldC, scale2, info2);
            // apply scale
            if (scale2[0] != 1)
                LAPACK(slascl)("G", i0, i0, s1, scale2, m, &n2, C_R, ldC, info);
        }
    }

    *scale = scale1[0] * scale2[0];
    *info = info1[0] || info2[0];
}