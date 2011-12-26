#!/usr/bin/tcc -run

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int
main (int argc, char *argv[])
{
	FILE *fp;
	char *buff;
	size_t buff_len, buff_size;
	int c, i;

	assert (argc == 2);

	fp = fopen (argv[1], "r");
	assert (fp != NULL);

	buff_len = 0;
	buff = malloc (buff_size = 8);

	while ((c = fgetc (fp)) != EOF)
	{
		if (buff_len >= buff_size)
			buff = realloc (buff, buff_size <<= 1);
		buff[buff_len++] = c;
	}
	
	for (i = 0; i < buff_len / 2; i++)
	{
		if (buff[i] != buff[buff_len - 1 - i])
		{
			puts ("Soubor neni palindromem.");
			return 0;
		}
	}

	puts ("Soubor je palindromem.");
	return 0;
}


