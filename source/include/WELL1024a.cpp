/* ***************************************************************************** */

/* Copyright:      Francois Panneton and Pierre L'Ecuyer, University of Montreal */

/*                 Makoto Matsumoto, Hiroshima University                        */

/* Notice:         This code can be used freely for personal, academic,          */

/*                 or non-commercial purposes. For commercial purposes,          */

/*                 please contact P. L'Ecuyer at: lecuyer@iro.UMontreal.ca       */

/* ***************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>
#include "WELL1024a.h"
#define W 32
#define R 32
#define M1 3
#define M2 24
#define M3 10
#define MAT0POS(t,v) (v^(v>>t))
#define MAT0NEG(t,v) (v^(v<<(-(t))))
#define Identity(v) (v)
#define V0            STATE[state_i                   ]
#define VM1           STATE[(state_i+M1) & 0x0000001fU]
#define VM2           STATE[(state_i+M2) & 0x0000001fU]
#define VM3           STATE[(state_i+M3) & 0x0000001fU]
#define VRm1          STATE[(state_i+31) & 0x0000001fU]
#define newV0         STATE[(state_i+31) & 0x0000001fU]
#define newV1         STATE[state_i                   ]
#define FACT 2.32830643653869628906e-10

static unsigned int state_i = 0;

static unsigned int STATE[R];

static unsigned int z0, z1, z2;

void InitWELLRNG1024a(unsigned int* init) {

    int j;

    state_i = 0;

    for (j = 0; j < R; j++)

        STATE[j] = init[j];

}

double WELLRNG1024a(void) {

    z0 = VRm1;

    z1 = Identity(V0) ^ MAT0POS(8, VM1);

    z2 = MAT0NEG(-19, VM2) ^ MAT0NEG(-14, VM3);

    newV1 = z1 ^ z2;

    newV0 = MAT0NEG(-11, z0) ^ MAT0NEG(-7, z1) ^ MAT0NEG(-13, z2);

    state_i = (state_i + 31) & 0x0000001fU;

    return ((double)STATE[state_i] * FACT);

}

int main(int argc, char **argv)
{
	if (argv[1] == NULL || argv[2] == NULL)
		return 1;

    int arg1 = atoi(argv[1]);
    int arg2 = atoi(argv[2]);

	if (argc <= 2)
		return 1;

    if (arg1 != 20201030)
        return 1;
	if (arg2 <= 0 || arg2 >= 2147483647)
		return 1;

	srand((unsigned)time(NULL));

	unsigned int init[32];
	for (int i = 0; i < 32; i++) {
		init[i] = rand() << 16 | rand();

	}
	InitWELLRNG1024a(init);
	printf("%d", (int)((double)WELLRNG1024a() * (arg2+1)));
	
	return 0;
}
