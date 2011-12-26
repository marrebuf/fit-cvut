#ifndef __PROGTEST__
	#include <stdio.h>
	#include <stdlib.h>
#endif /* ! __PROGTEST__ */

int
secondMin (int **mat, int size, int *min2)
{
	int i, k, n;
	int min[2], have_n = 0;

	for (i = 1; i < size; i++)
	for (k = 0; k < i; k++)
	{
		n = mat[i][k];

		if (have_n == 0)
			min[0] = n;
		else if (n < min[0])
		{
			min[1] = min[0];
			min[0] = n;
		}
		else if (have_n == 1)
			min[1] = n;
		else if (n < min[1])
			min[1] = n;

		have_n++;
	}

	if (have_n < 2)
		return 0;

	*min2 = min[1];
	return 1;
}

#ifndef __PROGTEST__
int
main (int argc, char *argv[])
{
	int ret, val;

	int m0r0[3] = {1, 4, 7};
	int m0r1[3] = {2, 5, 8};
	int m0r2[3] = {3, 6, 9};
	int *m0[3] = {m0r0, m0r1, m0r2};

	int m1r0[3] = {10, 20, 30};
	int m1r1[3] = {20, 30, 40};
	int m1r2[3] = {60, 50, 40};
	int *m1[3] = {m1r0, m1r1, m1r2};

	int m2r0[3] = {1, 2, 2};
	int m2r1[3] = {3, 1, 2};
	int m2r2[3] = {3, 3, 1};
	int *m2[3] = {m2r0, m2r1, m2r2};

	int m3r0[2] = {0, -5};
	int m3r1[2] = {3,  1};
	int *m3[2] = {m3r0, m3r1};

	int m4r0[1] = {100};
	int *m4[1] = {m4r0};

	ret = secondMin (m0, 3, &val);
	printf ("m0: ret = %d (ref: 1), val=%d (ref: 3)\n",  ret, val);

	ret = secondMin (m1, 3, &val);
	printf ("m1: ret = %d (ref: 1), val=%d (ref: 50)\n", ret, val);

	ret = secondMin (m2, 3, &val);
	printf ("m2: ret = %d (ref: 1), val=%d (ref: 3)\n",  ret, val);

	ret = secondMin (m3, 2, &val);
	printf ("m3: ret = %d (ref: 0)\n", ret);

	ret = secondMin (m4, 1, &val);
	printf ("m4: ret = %d (ref: 0)\n", ret);

	return 0;
}
#endif /* ! __PROGTEST__ */

