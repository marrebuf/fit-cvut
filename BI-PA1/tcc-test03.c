#!/usr/bin/tcc -run

#ifndef __PROGTEST__
	#include <stdio.h>
	#include <stdlib.h>
#endif /* __PROGTEST__ */

int
uniqueColumns (int ** mat, int rows, int cols, int * uniq)
{
	int i, k, l;

	if (cols < 1 || rows < 1)
		return 0;

	*uniq = 0;
	for (i = 0; i < cols; i++)
	{
		int local_top = 0;

		for (k = 0; k < cols; k++)
		{
			int same_vals = 0;

			for (l = 0; l < rows; l++)
				if (mat[l][i] == mat[l][k])
					same_vals++;

			if (same_vals == rows)
				local_top++;
		}
		if (local_top == 1)
			(*uniq)++;
	}

	return 1;
}

#ifndef __PROGTEST__
int
main (int argc, char *argv[])
{
	int ret, val;

	int m0r0[4] = {1, 2, 3, 4};
	int m0r1[4] = {1, 2, 3, 4};
	int m0r2[4] = {1, 2, 3, 4};
	int *m0[3] = {m0r0, m0r1, m0r2};

	int m1r0[5] = {1, 2, 1, 2, 1};
	int m1r1[5] = {3, 4, 3, 4, 5};
	int *m1[2] = {m1r0, m1r1};

	int m2r0[3] = {1, 1, 1};
	int m2r1[3] = {1, 1, 1};
	int m2r2[3] = {1, 1, 1};
	int *m2[3] = {m2r0, m2r1, m2r2};

	int m3r0[1] = {100};
	int *m3[1] = {m3r0};

	ret = uniqueColumns (m0, 3, 4, &val);
	printf ("m0: ret = %d (ref: 1), val=%d (ref: 4)\n", ret, val);

	ret = uniqueColumns (m1, 2, 5, &val);
	printf ("m1: ret = %d (ref: 1), val=%d (ref: 1)\n", ret, val);

	ret = uniqueColumns (m2, 3, 3, &val);
	printf ("m2: ret = %d (ref: 1), val=%d (ref: 0)\n", ret, val);

	ret = uniqueColumns (m3, 1, 1, &val);
	printf ("m3: ret = %d (ref: 1), val=%d (ref: 1)\n", ret, val);
	return 0;
}
#endif /* __PROGTEST__ */

