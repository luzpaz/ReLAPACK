#ifndef RELAPACK_INT_H
#define RELAPACK_INT_H

#include "../config.h"

#include "../inc/relapack.h"

#if FORTRAN_UNDERSCORE
#define FORTRAN(routine) routine ## _
#else
#define FORTRAN(routine) routine
#endif

#if BLAS_UNDERSCORE
#define BLAS(routine) routine ## _
#else
#define BLAS(routine) routine
#endif

#if LAPACK_UNDERSCORE
#define LAPACK(routine) routine ## _
#else
#define LAPACK(routine) routine
#endif

#include "lapack.h"
#include "blas.h"
#include "util.h"


// sytrf helper routines
void RELAPACK_sgemm_tr_rec(const char *, const char *, const char *, const int *, const int *, const float *, const float *, const int *, const float *, const int *, const float *, float *, const int *);
void RELAPACK_dgemm_tr_rec(const char *, const char *, const char *, const int *, const int *, const double *, const double *, const int *, const double *, const int *, const double *, double *, const int *);
void RELAPACK_cgemm_tr_rec(const char *, const char *, const char *, const int *, const int *, const float *, const float *, const int *, const float *, const int *, const float *, float *, const int *);
void RELAPACK_zgemm_tr_rec(const char *, const char *, const char *, const int *, const int *, const double *, const double *, const int *, const double *, const int *, const double *, double *, const int *);

void FORTRAN(relapack_ssytrf_rec2)(const char *, const int *, const int *, int *, float *, const int *, int *, float *, const int *, int *);
void FORTRAN(relapack_dsytrf_rec2)(const char *, const int *, const int *, int *, double *, const int *, int *, double *, const int *, int *);
void FORTRAN(relapack_csytrf_rec2)(const char *, const int *, const int *, int *, float *, const int *, int *, float *, const int *, int *);
void FORTRAN(relapack_chetrf_rec2)(const char *, const int *, const int *, int *, float *, const int *, int *, float *, const int *, int *);
void FORTRAN(relapack_zsytrf_rec2)(const char *, const int *, const int *, int *, double *, const int *, int *, double *, const int *, int *);
void FORTRAN(relapack_zhetrf_rec2)(const char *, const int *, const int *, int *, double *, const int *, int *, double *, const int *, int *);
void FORTRAN(relapack_ssytrf_rook_rec2)(const char *, const int *, const int *, int *, float *, const int *, int *, float *, const int *, int *);
void FORTRAN(relapack_dsytrf_rook_rec2)(const char *, const int *, const int *, int *, double *, const int *, int *, double *, const int *, int *);
void FORTRAN(relapack_csytrf_rook_rec2)(const char *, const int *, const int *, int *, float *, const int *, int *, float *, const int *, int *);
void FORTRAN(relapack_chetrf_rook_rec2)(const char *, const int *, const int *, int *, float *, const int *, int *, float *, const int *, int *);
void FORTRAN(relapack_zsytrf_rook_rec2)(const char *, const int *, const int *, int *, double *, const int *, int *, double *, const int *, int *);
void FORTRAN(relapack_zhetrf_rook_rec2)(const char *, const int *, const int *, int *, double *, const int *, int *, double *, const int *, int *);

// trsyl helper routines
void FORTRAN(relapack_strsyl_rec2)(const char *, const char *, const int *, const int *, const int *, const float *, const int *, const float *, const int *, float *, const int *, float *, int *);
void FORTRAN(relapack_dtrsyl_rec2)(const char *, const char *, const int *, const int *, const int *, const double *, const int *, const double *, const int *, double *, const int *, double *, int *);
void FORTRAN(relapack_ctrsyl_rec2)(const char *, const char *, const int *, const int *, const int *, const float *, const int *, const float *, const int *, float *, const int *, float *, int *);
void FORTRAN(relapack_ztrsyl_rec2)(const char *, const char *, const int *, const int *, const int *, const double *, const int *, const double *, const int *, double *, const int *, double *, int *);

#endif /*  RELAPACK_INT_H */
