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
		
	float *A1 = malloc(n * n * sizeof(float));
	float *A2 = malloc(n * n * sizeof(float));

    // Ouptut
    int info;

    { // L
        // generate matrix
        s2matgen(n, n, A1, A2);

        // run
        RELAPACK(slauum)("L", &n, A1, &n, &info);
        LAPACK(slauu2)("L", &n, A2, &n, &info);

        // check error
        const double error = s2vecerr(n * n, A1, A2);
        printf("slauum L:\t%g\n", error);
    }

    { // U
        // generate matrix
        s2matgen(n, n, A1, A2);

        // run
        RELAPACK(slauum)("U", &n, A1, &n, &info);
        LAPACK(slauu2)("U", &n, A2, &n, &info);

        // check error
        const double error = s2vecerr(n * n, A1, A2);
        printf("slauum U:\t%g\n", error);
    }

    free(A1);
    free(A2);

	return 0;
}
