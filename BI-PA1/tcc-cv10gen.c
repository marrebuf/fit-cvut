#!/usr/bin/tcc -run

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

static char *
gen_palindrom (int len)
{
	char *buff;
	int i;

	assert (len > 0);

	buff = malloc (len + 1);
	assert (buff != NULL);

	for (i = 0; i < (len + 1) / 2; i++)
		buff[i] = buff[len - i - 1] = 'a' + (rand () % 26);

	buff[len] = '\0';
	return buff;
}

int
main (int argc, char *argv[])
{
	int len;

	assert (argc == 2);
	len = atoi (argv[1]);

	srand (time (NULL));
	fputs (gen_palindrom (len), stdout);
	return 0;
}

