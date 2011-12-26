#ifndef __PROGTEST__
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <ctype.h>
#endif /* ! __PROGTEST__ */

static int
max (int a, int b)
{
	return (a > b) ? a : b;
}

int
sumBribes (int budget, int *table, int tableLen)
{
	int *m, i, j, result;

	m = (int *) calloc (sizeof *table, budget + 1);
	for (j = 1; j <= budget; j++)
	{
		int maxItems = 0;
		for (i = 1; i < tableLen && i <= j; i++)
			if (table[i])
				maxItems = max (maxItems, m[j - i] + table[i]);

		m[j] = max (m[j - 1], maxItems);
	}

	result = m[budget];
	free (m);
	return result;
}

#ifndef __PROGTEST__
static void
test (int budget, int *table, int table_len, int result)
{
	static int nr;
	int r;

	printf ("-- Test %d: ", ++nr);
	r = sumBribes (budget, table, table_len);
	if (r != result)
	{
		puts ("FAIL");
		printf ("Should be: %d\n", result);
		printf ("Returned:  %d\n", r);
	}
	else
		puts ("OK");
}

int
main (int argc, char *argv[])
{
	int test1[] = {0, 100, 0, 0, 350, 0, 750};
	int test2[] = {0, 1, 7, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 53};
	int test3[] = {0, 0, 0, 0, 0, 0, 0, 1};

	test (1,  test1, 7, 100);
	test (1,  test1, 7, 100);
	test (2,  test1, 7, 200);
	test (3,  test1, 7, 300);
	test (4,  test1, 7, 400);
	test (5,  test1, 7, 500);
	test (6,  test1, 7, 750);
	test (7,  test1, 7, 850);
	test (8,  test1, 7, 950);
	test (9,  test1, 7, 1050);
	test (10, test1, 7, 1150);
	test (11, test1, 7, 1250);
	test (12, test1, 7, 1500);
	test (13, test1, 7, 1600);
	test (14, test1, 7, 1700);
	test (15, test1, 7, 1800);
	test (16, test1, 7, 1900);
	test (17, test1, 7, 2000);

	test (1,  test2, 16, 1);
	test (2,  test2, 16, 7);
	test (3,  test2, 16, 8);
	test (4,  test2, 16, 14);
	test (5,  test2, 16, 15);
	test (6,  test2, 16, 21);
	test (7,  test2, 16, 22);
	test (8,  test2, 16, 28);
	test (9,  test2, 16, 29);
	test (10, test2, 16, 35);
	test (11, test2, 16, 36);
	test (12, test2, 16, 42);
	test (13, test2, 16, 43);
	test (14, test2, 16, 49);
	test (15, test2, 16, 53);
	test (16, test2, 16, 56);
	test (17, test2, 16, 60);

	test (1,  test3, 8, 0);
	test (2,  test3, 8, 0);
	test (3,  test3, 8, 0);
	test (4,  test3, 8, 0);
	test (5,  test3, 8, 0);
	test (6,  test3, 8, 0);
	test (7,  test3, 8, 1);
	test (8,  test3, 8, 1);
	test (9,  test3, 8, 1);
	test (10, test3, 8, 1);
	test (11, test3, 8, 1);
	test (12, test3, 8, 1);
	test (13, test3, 8, 1);
	test (14, test3, 8, 2);
	test (15, test3, 8, 2);
	test (16, test3, 8, 2);
	test (17, test3, 8, 2);

	return 0;
}
#endif /* ! __PROGTEST__ */

