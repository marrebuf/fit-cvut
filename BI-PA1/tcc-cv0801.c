#!/usr/bin/tcc -run

#include <stdio.h>
#include <stdlib.h>

#define aliquot_sum(n) (1 + sum_table[n - 1])

static void
print_abundant (int limit)
{
	int i, k, s, *sum_table;

	/* Generate a table of aliquot sums. */
	sum_table = (int *) calloc (limit, sizeof (int));

	*sum_table = -1;
	for (i = 2; i <= limit; i++)
		for (k = 2 * i; k <= limit; k += i)
			sum_table[k - 1] += i;

	/* Find amicable numbers within the given interval. */
	for (i = 1; i <= limit; i++)
	{
		s = aliquot_sum (i);
		if (s > i)
			printf ("%d (%d)\n", i, s);
	}

	free (sum_table);
}

int
main (int argc, char *argv[])
{
	int r, limit;

	puts ("Zadejte limit:");
	r = scanf ("%d", &limit);
	if (r != 1 || limit <= 0)
		exit (1);

	print_abundant (limit);
	return 0;
}

