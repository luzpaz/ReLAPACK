#include "../../config.h"
#include "../blas.h"
#include "../lapack.h"

void strsyl_rnt(const int *isgn, const int *m, const int *n, const float *A, const int *ldA, const float *B, const int *ldB, float *C, const int *ldC, const float *scale) {
    if (*m <= LARPACK_CROSSOVER && *n <= LARPACK_CROSSOVER) { 
        int info;
        LAPACK(strsl2)("N", "T", isgn, m, n, A, ldA, B, ldB, C, ldC, &info);
        return;
    }
    
   	const float s1[] = {1}, sm1[] = {-1}, smisgn[] = {-*isgn};

    if (*m > *n) {
        const int m1 = (*m >= 16) ? ((*m + 8) / 16) * 8 : *m / 2;
        const int m2 = *m - m1;

        // A_TL A_TR
        //      A_BR
    float *const A_TL = A;
    float *const A_TR = A + *ldA * m2;
    float *const A_BR = A + *ldA * m2 + m2;

        // C_T
        // C_B
    float *const C_T = C;
    float *const C_B = C + m2;

        // C_B = sylv(A_BR, B', C_B)
        strsyl_rnt(isgn, &m1, n, A_BR, ldA, B, ldB, C_B, ldC, scale);
        // C_T = C_T - A_TR * C_B
        BLAS(sgemm)("N", "N", &m2, n, &m1, sm1, A_TR, ldA, C_B, ldC, s1, C_T, ldC);
        // C_T = sylv(A_TL, B', C_T)
        strsyl_rnt(isgn, &m2, n, A_TL, ldA, B, ldB, C_T, ldC, scale);
    } else {
        const int n1 = (*n >= 16) ? ((*n + 8) / 16) * 8 : *n / 2;
        const int n2 = *n - n1;

        // B_TL B_TR
        //      B_BR
    float *const B_TL = B;
    float *const B_TR = B + *ldB * n2;
    float *const B_BR = B + *ldB * n2 + n2;

        // C_L C_R
    float *const C_L = C;
    float *const C_R = C + *ldC * n2;

        // C_R = sylv(A, B_BR', C_R)
        strsyl_rnt(isgn, m, &n2, A, ldA, B_BR, ldB, C_R, ldC, scale);
        // C_L = C_L -/+ C_R * B_TR'
        BLAS(sgemm)("N", "T", m, &n2, &n1, smisgn, C_R, ldC, B_TR, ldB, s1, C_L, ldC);
        // C_L = sylv(A, B_TL', C_L)
        strsyl_rnt(isgn, m, &n1, A, ldA, B_TL, ldB, C_L, ldC, scale);
    }
}