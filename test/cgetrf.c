#include "../src/relapack.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]) {

    if (argc == 1) {
        fprintf(stderr, "usage: %s n\n", argv[0]);
        return 0;
    }
    const int n  = atoi(argv[1]);
    const int n2 = (n * 3) / 4;
		
	float *A1    = malloc(n * n * 2 * sizeof(float));
	float *A2    = malloc(n * n * 2 * sizeof(float));
    int   *ipiv1 = malloc(n * sizeof(int));
    int   *ipiv2 = malloc(n * sizeof(int));

    // Output
    int info;

    { // m = n
        // generate matrix
        c2matgen(n, n, A1, A2);

        // run
        RELAPACK(cgetrf)(&n, &n, A1, &n, ipiv1, &info);
        LAPACK(cgetf2)(&n, &n, A2, &n, ipiv2, &info);

        // check error
        const double error = c2vecerr(n * n, A1, A2) + i2vecerr(n, ipiv1, ipiv2);
        printf("cgetrf m = n:\t%g\n", error);
    }

    { // m > n
        // generate matrix
        c2matgen(n, n2, A1, A2);

        // run
        RELAPACK(cgetrf)(&n, &n2, A1, &n, ipiv1, &info);
        LAPACK(cgetf2)(&n, &n2, A2, &n, ipiv2, &info);

        // check error
        const double error = c2vecerr(n * n2, A1, A2) + i2vecerr(n2, ipiv1, ipiv2);
        printf("cgetrf m > n:\t%g\n", error);
    }

    { // m < n
        // generate matrix
        c2matgen(n2, n, A1, A2);

        // run
        RELAPACK(cgetrf)(&n2, &n, A1, &n, ipiv1, &info);
        LAPACK(cgetf2)(&n2, &n, A2, &n, ipiv2, &info);

        // check error
        const double error = c2vecerr(n2 * n, A1, A2) + i2vecerr(n2, ipiv1, ipiv2);
        printf("cgetrf m < n:\t%g\n", error);
    }

    free(A1);
    free(A2);
    free(ipiv1);
    free(ipiv2);

	return 0;
}
