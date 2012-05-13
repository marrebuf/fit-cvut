#!/usr/bin/tcc -run

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>

#define X 0
#define Y 1

int
main (int argc, char *argv[])
{
	int r;
	double pt[2], p1[2], p2[2];
	double dp[2], d[2];
	double diff, e;

	printf ("Zadej bod [x y]: ");
	r = scanf ("%lf %lf", &pt[X], &pt[Y]);
	assert (r == 2);

	printf ("Zadej usecku [x1 y1 x2 y2]: ");
	r = scanf ("%lf %lf %lf %lf", &p1[X], &p1[Y], &p2[X], &p2[Y]);
	assert (r == 4);

	if (p1[X] > p2[X])
	{
		r = p2[X];
		p2[X] = p1[X];
		p1[X] = r;
	}

	if (p1[Y] > p2[Y])
	{
		r = p2[Y];
		p2[Y] = p1[Y];
		p1[Y] = r;
	}

	d[X] = p2[X] - p1[X];
	d[Y] = p2[Y] - p1[Y];

	dp[X] = pt[X] - p1[X];
	dp[Y] = pt[Y] - p1[Y];

	diff = dp[X] / d[X] * d[Y] - dp[Y];
	e = DBL_EPSILON * 64;

	printf ("Bod %s na usecce.\n",
		(diff < e && pt[X] >= p1[X] && pt[X] <= p2[X]
		          && pt[Y] >= p1[Y] && pt[Y] <= p2[Y])
		      ? "lezi"
		      : "nelezi");

	return 0;
}

