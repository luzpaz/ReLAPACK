#include "../src/relapack.h"
#include "util.h"
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]) {

    if (argc == 1) {
        fprintf(stderr, "usage: %s n\n", argv[0]);
        return 0;
    }
    const int n = atoi(argv[1]);
		
	float *A1 = malloc(2 * n * n * sizeof(float));
	float *A2 = malloc(2 * n * n * sizeof(float));

    // Output
    int info;

    { // L
        // generate matrix
        c2matgen(n, n, A1, A2);

        // run
        RELAPACK(ctrtri)("L", "N", &n, A1, &n, &info);
        LAPACK(ctrti2)("L", "N", &n, A2, &n, &info);

        // check error
        const double error = c2vecerr(n * n, A1, A2);
        printf("ctrtri L:\t%g\n", error);
    }

    { // U
        // generate matrix
        c2matgen(n, n, A1, A2);

        // run
        RELAPACK(ctrtri)("U", "N", &n, A1, &n, &info);
        LAPACK(ctrti2)("U", "N", &n, A2, &n, &info);

        // check error
        const double error = c2vecerr(n * n, A1, A2);
        printf("ctrtri U:\t%g\n", error);
    }

    free(A1);
    free(A2);

	return 0;
}
