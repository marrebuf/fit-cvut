#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define CHECK(cond) \
	if (!(cond)) { \
		puts ("Nespravny vstup."); \
		exit (EXIT_FAILURE); \
	}

#define HX(n) houses[2 * (n)]
#define HY(n) houses[2 * (n) + 1]

static int *houses;
static unsigned houses_alloc, n_houses;

static void
houses_free (void)
{
	if (houses)
		free (houses);
}

static void
houses_init (void)
{
	atexit (houses_free);
	houses = (int *) malloc (sizeof (int) * 2
		* (houses_alloc = 4096));
	assert (houses != NULL);
}

static void
houses_check_size (void)
{
	if (n_houses != houses_alloc)
		return;

	houses = (int *) realloc (houses, sizeof (int) * 2
		* (houses_alloc += 4096));
	assert (houses != NULL);
}

static int
sort_by_x (const void *first, const void *second)
{
	return ((int *) first)[0] - ((int *) second)[0];
}

static int
sort_by_y (const void *first, const void *second)
{
	return ((int *) first)[1] - ((int *) second)[1];
}

int
main (int argc, char *argv[])
{
	int i, i_max;
	long int len = 0;

	houses_init ();

	/* Read in some actual houses. */
	puts ("Zadejte souradnice domu:");

	while (1)
	{
		i = scanf ("%d %d",
			&HX (n_houses),
			&HY (n_houses));

		if (i == EOF)
			break;
		CHECK (i == 2);

		++n_houses;
		houses_check_size ();
	}
	CHECK (n_houses > 0);

	/* Compute the shortest length. */
	i_max = n_houses / 2;

	qsort (houses, n_houses, sizeof (int) * 2, sort_by_x);
	for (i = 0; i < i_max; i++)
		len += HX (n_houses - 1 - i) - HX (i);

	qsort (houses, n_houses, sizeof (int) * 2, sort_by_y);
	for (i = 0; i < i_max; i++)
		len += HY (n_houses - 1 - i) - HY (i);

	printf ("Delka rozvodu: %ld\n", len);
	return 0;
}

