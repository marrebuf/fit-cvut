#ifndef __PROGTEST__
	#include <stdio.h>
	#include <stdlib.h>
	#include <math.h>
#endif /* ! __PROGTEST__ */

/** Compute the area divided by stones @a s1 and @a s2. */
static double
computeArea (double x[], double y[], int n, int s1, int s2)
{
	int i;
	double cx, cy, vx1, vy1, vx2, vy2;
	double area = 0;

	/* The center is our point A. */
	cx = (x[s1] + x[s2]) / 2.;
	cy = (y[s1] + y[s2]) / 2.;

	/* Compute the vector AB. */
	vx1 = x[s1] - cx;
	vy1 = y[s1] - cy;

	for (i = (s1 + 1) % n; ; i = (i + 1) % n)
	{
		/* Compute the vector AC. */
		vx2 = x[i] - cx;
		vy2 = y[i] - cy;

		/* Add this triangle to the sum. */
		area += fabs (vx1 * vy2 - vx2 * vy1);

		if (i == s2)
			break;

		/* Move on. AC is now AB. */
		vx1 = vx2;
		vy1 = vy2;
	}
	return area / 2.;
}

/** Compute the difference of area of the two halves
 *  specified by @a s1 and @a s2.
 */
#define computeDiff(x, y, n, s1, s2) \
	fabs (2 * computeArea ((x), (y), (n), (s1), (s2)) - area)

/* Wrap around the ends of our value arrays. */
#define wrap_down(i) (((i) + n) % n)
#define wrap_up(i)   ( (i)      % n)

/* It could even be like so:
 * #define wrap_down(i) ((i) <  0 ? (i) + n : (i))
 * #define wrap_up(i)   ((i) >= n ? (i) - n : (i))
 * But which one is better? Slow division or jumps?
 */

int
divideLand (double x[], double y[], int n, double *diff)
{
	int first, second, i;
	double min_diff = HUGE_VAL, ref_diff, test_diff;
	double area;

	if (n < 4)
		return 0;

	area = computeArea (x, y, n, 0, 0);

	for (first = 0; first < n; first++)
	{
		/* Hopefully the opposite stone. */
		/* Pure heuristics, dear Watson. */
		second = (first + (n >> 1)) % n;

		/* Pick this opposite median as a reference. */
		ref_diff = computeDiff (x, y, n, first, second);

		/* Search for smaller differences counterclockwise. */
		for (i = second - 1; ; i--)
		{
			if (wrap_down (i - 1) == first)
				break;
			i = wrap_down (i);

			test_diff = computeDiff (x, y, n, first, i);
			if (test_diff > ref_diff)
				break;

			ref_diff = test_diff;
		}

		/* Search for smaller differences clockwise. */
		for (i = second + 1; ; i++)
		{
			if (wrap_up (i + 1) == first)
				break;
			i = wrap_up (i);

			test_diff = computeDiff (x, y, n, first, i);
			if (test_diff > ref_diff)
				break;

			ref_diff = test_diff;
		}

		/* Have we got a better result? Great. */
		if (min_diff > ref_diff)
			min_diff = ref_diff;
	}

	*diff = min_diff;
	return 1;
}

#ifndef __PROGTEST__
static void
test (double *x, double *y, int n, int r, double diff)
{
	static int test_nr = 0;
	int my_r;
	double my_diff = NAN;
	double area;

	++test_nr;
	area = computeArea (x, y, n, 0, 0);

	printf ("Test %2d: ", test_nr);
	my_r = divideLand (x, y, n, &my_diff);
	if (my_r == r && (!r || fabs (my_diff - diff) <= 1e-10 * area))
		puts ("OK");
	else
		printf ("FAIL: %14f, expected %14f (delta %g)\n",
			my_diff, diff, fabs (my_diff - diff));
}

int
main (int argc, char *argv[])
{
	/* Reference values. */
	double x0[4] = {0, 50, 60, 0};
	double y0[4] = {0, 0, 60, 50};
	test (x0, y0, 4, 1, 0.000000);

	double x1[5] = {0, 5, 15, 20, 10};
	double y1[5] = {10, 0, 0, 10, 15};
	test (x1, y1, 5, 1, 75.000000);

	double x2[3] = {5, 15, 10};
	double y2[3] = {0, 0, 15};
	test (x2, y2, 3, 0, NAN);

	return 0;
}
#endif /* ! __PROGTEST__ */

