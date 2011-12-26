#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define CHECK(cond) \
	if (!(cond)) { \
		puts ("Nespravny vstup."); \
		exit (EXIT_FAILURE); \
	}

/** Get the aliquot sum for @a n from our lookup table. */
#define aliquot_sum(n) (1 + sum_table[n - 1])

int
main (int argc, char *argv[])
{
	int limit, n, s;
	int i, k, *sum_table;

	puts ("Zadejte limit:");
	CHECK (scanf ("%d", &limit) == 1);
	CHECK (limit > 0);

	/* Generate a table of aliquot sums. */
	sum_table = (int *) calloc (limit, sizeof (int));
	assert (sum_table != NULL);

	*sum_table = -1;
	for (i = 2; i <= limit; i++)
		for (k = 2 * i; k <= limit; k += i)
			sum_table[k - 1] += i;

	/* Find amicable numbers within the given interval. */
	for (n = 220; n <= limit; n++)
	{
		s = aliquot_sum (n);
		if (s > n && s <= limit && aliquot_sum (s) == n)
			printf ("%d, %d\n", n, s);
	}

	return 0;
}

