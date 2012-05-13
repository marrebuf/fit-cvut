#!/usr/bin/tcc -run

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

static void
load_str (char *a, size_t size)
{
	char *p;

	if (!fgets (a, size, stdin))
		exit (1);

	p = strchr (a, '\n');
	if (p)
		*p = '\0';
}

static int
same_letters (const char *s1, const char *s2)
{
	int c[256] = {0};
	const char *p;

	for (p = s1; *p; p++)
		c[tolower (*p)] = 1;
	for (p = s2; *p; p++)
		if (!c[tolower (*p)])
			return 0;

	return 1;
}

int
main (int argc, char *argv[])
{
	char s1[160], s2[160];

	puts ("Zadejte 2 řetězce:");

	load_str (s1, sizeof s1);
	load_str (s2, sizeof s2);

	if (same_letters (s1, s2))
		puts ("Jsou ze stejných písmen.");
	else
		puts ("Nejsou ze stejných písmen.");
	
	return 0;
}

