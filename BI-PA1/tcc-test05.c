#!/usr/bin/tcc -run

#ifndef __PROGTEST__
	#include <stdio.h>
	#include <stdlib.h>
#endif /* __PROGTEST__ */

int
secondMax (int **mat, int size, int *max2)
{
	int i, k, n;
	int max[2], have_n = 0;

	for (i = 0; i < size - 1; i++)
	for (k = i + 1; k < size; k++)
	{
		n = mat[i][k];

		if (have_n == 0)
			max[0] = n;
		else if (n > max[0])
		{
			max[1] = max[0];
			max[0] = n;
		}
		else if (have_n == 1)
			max[1] = n;
		else if (n > max[1])
			max[1] = n;

		have_n++;
	}

	if (have_n < 2)
		return 0;

	*max2 = max[1];
	return 1;
}

#ifndef __PROGTEST__
int
main (int argc, char *argv[])
{
	int ret, val;

	int m0r0[3] = {1, 2, 3};
	int m0r1[3] = {4, 5, 6};
	int m0r2[3] = {7, 8, 9};
	int *m0[3] = {m0r0, m0r1, m0r2};

	int m1r0[3] = {10, 20, 30};
	int m1r1[3] = {24, 25, 26};
	int m1r2[3] = {27, 28, 29};
	int *m1[3] = {m1r0, m1r1, m1r2};

	int m2r0[3] = {1, 2, 2};
	int m2r1[3] = {3, 1, 2};
	int m2r2[3] = {3, 3, 1};
	int *m2[3] = {m2r0, m2r1, m2r2};

	int m3r0[2] = {0, -5};
	int m3r1[2] = {3, 1};
	int *m3[2] = {m3r0, m3r1};

	int m4r0[1] = {100};
	int *m4[1] = {m4r0};

	ret = secondMax (m0, 3, &val);
	printf ("m0: ret = %d (ref: 1), val=%d (ref: 3)\n", ret, val);

	ret = secondMax (m1, 3, &val);
	printf ("m1: ret = %d (ref: 1), val=%d (ref: 26)\n", ret, val);

	ret = secondMax (m2, 3, &val);
	printf ("m2: ret = %d (ref: 1), val=%d (ref: 2)\n", ret, val);

	ret = secondMax (m3, 2, &val);
	printf ("m3: ret = %d (ref: 0)\n", ret);

	ret = secondMax (m4, 1, &val);
	printf ("m4: ret = %d (ref: 0)\n", ret);
	return 0;
}
#endif /* __PROGTEST__ */

