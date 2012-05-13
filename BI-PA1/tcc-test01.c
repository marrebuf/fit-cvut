#!/usr/bin/tcc -run

#ifndef __PROGTEST__
	#include <stdio.h>
	#include <stdlib.h>
#endif /* __PROGTEST__ */

int
mostFrequentRow (int **mat, int rows, int cols, int *occ)
{
	int i, k, l;

	if (rows < 2)
		return 0;

	*occ = 0;
	for (i = 0; i < rows; i++)
	{
		int local_top = 0;

		for (k = i; k < rows; k++)
		{
			int same_vals = 0;

			for (l = 0; l < cols; l++)
				if (mat[i][l] == mat[k][l])
					same_vals++;

			if (same_vals == cols)
				local_top++;
		}
		if (local_top > *occ)
			*occ = local_top;
	}

	return 1;
}

#ifndef __PROGTEST__
int
main (int argc, char *argv[])
{
	int ret, val;

	int m0r0[3] = {1, 4, 7};
	int m0r1[3] = {2, 5, 8};
	int m0r2[3] = {1, 4, 7};
	int *m0[3] = {m0r0, m0r1, m0r2};

	int m1r0[3] = {10, 20, 30};
	int m1r1[3] = {20, 30, 40};
	int m1r2[3] = {10, 20, 30};
	int m1r3[3] = {10, 20, 30};
	int m1r4[3] = {20, 30, 40};
	int *m1[5] = {m1r0, m1r1, m1r2, m1r3, m1r4};

	int m2r0[3] = {1, 1, 1};
	int m2r1[3] = {1, 2, 1};
	int m2r2[3] = {1, 2, 2};
	int *m2[3] = {m2r0, m2r1, m2r2};

	int m3r0[1] = {100};
	int *m3[1] = {m3r0};

	ret = mostFrequentRow (m0, 3, 3, &val);
	printf ("m0: ret = %d (ref: 1), val=%d (ref: 2)\n", ret, val);

	ret = mostFrequentRow (m1, 5, 3, &val);
	printf ("m1: ret = %d (ref: 1), val=%d (ref: 3)\n", ret, val);

	ret = mostFrequentRow (m2, 3, 3, &val);
	printf ("m2: ret = %d (ref: 1), val=%d (ref: 1)\n", ret, val);

	ret = mostFrequentRow (m3, 1, 1, &val);
	printf ("m3: ret = %d (ref: 0)\n", ret);
	return 0;
}
#endif /* __PROGTEST__ */

