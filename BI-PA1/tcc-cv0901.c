#!/usr/bin/tcc -run

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define ARR_LEN 10

#define show_array \
	for (i = 0; i < ARR_LEN; i++) \
		printf ("%d ", arr[i]); \
	printf ("\n");

static void
sort (int *arr, size_t arr_len)
{
	int *p1, *p2, tmp, pivot;

	if (arr_len < 1)
		return;

	p1 = arr;
	p2 = arr + ARR_LEN - 1;

	pivot = *p2;

	while (1)
	{
		while (*p1 > pivot && p1 < p2)
			p1++;
		while (*p2 < pivot && p2 > p1)
			p2--;

		if (p1 == p2)
			break;

		tmp = *p1;
		*p1 = *p2;
		*p2 = tmp;
	}

	sort (arr, p1 - arr);
	sort (p2, arr + arr_len - p2);
}

int
main (int argc, char *argv[])
{
	int i, arr[ARR_LEN];

	srand (time (NULL));

	for (i = 0; i < ARR_LEN; i++)
		arr[i] = rand () % 100;

	show_array
	sort (arr, ARR_LEN);
	show_array

	return 0;
}

