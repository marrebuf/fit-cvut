/** This implementation hasn't received the full number of score points;
 *  there were more people that couldn't find the problem.
 */

#ifndef __PROGTEST__
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <ctype.h>
#endif /* __PROGTEST__ */

static void
append (char **buffer, int *r_len, int *r_alloc, char *str, int str_len)
{
	if (*r_len + str_len >= *r_alloc)
	{
		*r_alloc = *r_len + str_len + 1000;
		*buffer = (char *) realloc (*buffer, *r_alloc);
	}

	memcpy (*buffer + *r_len, str, str_len);
	*r_len += str_len;
}

char *
wordWrap (char *str, int width)
{
	char *r = 0, *marker = 0;
	int r_len = 0, r_alloc = 0;

	int line_len = 0, last_is_nl = 0;

	for (; *str; str++)
	{
		if (isspace (*str))
		{
			if (marker)
			{
				if (str - marker > width)
				{
					free (r);
					return 0;
				}

				if (line_len + str - marker + !!line_len > width)
				{
					append (&r, &r_len, &r_alloc, (char *) "\n", 1);
					line_len = 0;
				}
				else if (line_len)
				{
					append (&r, &r_len, &r_alloc, (char *) " ", 1);
					line_len += 1;
				}

				append (&r, &r_len, &r_alloc, marker, str - marker);
				line_len += str - marker;

				marker = 0;
			}
		}
		else if (!marker)
			marker = str;

		if (*str == '\n')
			last_is_nl += 1;
		else if (!isspace (*str))
		{
			if (last_is_nl >= 2)
			{
				append (&r, &r_len, &r_alloc, (char *) "\n\n", 2);
				line_len = 0;
			}
			last_is_nl = 0;
		}
	}

	if (r_len)
		append (&r, &r_len, &r_alloc, (char *) "\n", 1);

	append (&r, &r_len, &r_alloc, (char *) "\0", 1);
	return r;
}
  
#ifndef __PROGTEST__
static void
test (const char *a, const char *b)
{
	if (!a)
		puts ("-- No output");
	else if (strcmp (a, b))
	{
		puts ("-- Wrong conversion");
		puts (a);
	}
}

int
main (int argc, char *argv[])
{
	char *r, *str = (char *)
		"Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Integer metus\n"
		"pede, pretium vitae, rhoncus et, auctor sit amet, ligula. Integer volutpat\n"
		"orci et elit. Nunc tempus, urna at sollicitudin rutrum, arcu libero rhoncus\n"
		"lectus, vitae feugiat purus orci ultricies turpis. Pellentesque habitant\n"
		"morbi tristique senectus et netus et malesuada fames ac turpis egestas. Nam\n"
		"in pede. Etiam eu sem id urna ultricies congue. Vestibulum porttitor\n"
		"ultrices neque. Mauris semper, mauris ut feugiat ultricies, augue purus\n"
		"tincidunt  elit, eu interdum ante nisl ac ante. Pellentesque dui. Vestibulum\n"
		"pretium, augue non cursus pretium, nibh dolor laoreet leo, sed pharetra pede\n"
		"libero non diam.\n"
		"\n"
		"Proin est nisi,                     gravida ac, vulputate id, fringilla sit\n"
		"amet, magna. Nam congue cursus magna. In malesuada, velit a gravida sodales,\n"
		"dolor nisl vestibulum orci, sit amet sagittis mauris tellus nec purus. Nulla\n"
		"eget risus. Quisque nec sapien blandit odio convallis ullamcorper. Lorem\n"
		"ipsum dolor sit amet, consectetuer adipiscing elit. Pellentesque cursus.\n"
		"Aliquam tempus neque vitae libero molestie ut auctor.\n"
		"\n"
		"\n"
		"\n"
		"In nec massa eu tortor vulputate suscipit. Nam tristique magna nec pede. Sed\n"
		"a nisi. Nulla sed augue ut risus placerat porttitor. Ut aliquam. Nulla\n"
		"facilisi. Nulla vehicula nibh ac sapien. Nunc facilisis dapibus ipsum. Donec\n"
		"sed mauris. Nulla quam nisi, laoreet non, dignissim posuere, lacinia nec,\n"
		"turpis. Mauris malesuada nisi sed enim. In hac habitasse platea dictumst.\n"
		"Fusce    faucibus, turpis nec auctor posuere, nulla tellus scelerisque metus,\n"
		"quis molestie mi dui id quam. Mauris vestibulum. Nam ullamcorper.\n"
		"";

	r = wordWrap (str, 10);
	if (r != NULL)
		puts ("-- Not NULL");

	r = wordWrap (str, 40);
	test (r,
		"Lorem ipsum dolor sit amet, consectetuer\n"
		"adipiscing elit. Integer metus pede,\n"
		"pretium vitae, rhoncus et, auctor sit\n"
		"amet, ligula. Integer volutpat orci et\n"
		"elit. Nunc tempus, urna at sollicitudin\n"
		"rutrum, arcu libero rhoncus lectus,\n"
		"vitae feugiat purus orci ultricies\n"
		"turpis. Pellentesque habitant morbi\n"
		"tristique senectus et netus et malesuada\n"
		"fames ac turpis egestas. Nam in pede.\n"
		"Etiam eu sem id urna ultricies congue.\n"
		"Vestibulum porttitor ultrices neque.\n"
		"Mauris semper, mauris ut feugiat\n"
		"ultricies, augue purus tincidunt elit,\n"
		"eu interdum ante nisl ac ante.\n"
		"Pellentesque dui. Vestibulum pretium,\n"
		"augue non cursus pretium, nibh dolor\n"
		"laoreet leo, sed pharetra pede libero\n"
		"non diam.\n"
		"\n"
		"Proin est nisi, gravida ac, vulputate\n"
		"id, fringilla sit amet, magna. Nam\n"
		"congue cursus magna. In malesuada, velit\n"
		"a gravida sodales, dolor nisl vestibulum\n"
		"orci, sit amet sagittis mauris tellus\n"
		"nec purus. Nulla eget risus. Quisque nec\n"
		"sapien blandit odio convallis\n"
		"ullamcorper. Lorem ipsum dolor sit amet,\n"
		"consectetuer adipiscing elit.\n"
		"Pellentesque cursus. Aliquam tempus\n"
		"neque vitae libero molestie ut auctor.\n"
		"\n"
		"In nec massa eu tortor vulputate\n"
		"suscipit. Nam tristique magna nec pede.\n"
		"Sed a nisi. Nulla sed augue ut risus\n"
		"placerat porttitor. Ut aliquam. Nulla\n"
		"facilisi. Nulla vehicula nibh ac sapien.\n"
		"Nunc facilisis dapibus ipsum. Donec sed\n"
		"mauris. Nulla quam nisi, laoreet non,\n"
		"dignissim posuere, lacinia nec, turpis.\n"
		"Mauris malesuada nisi sed enim. In hac\n"
		"habitasse platea dictumst. Fusce\n"
		"faucibus, turpis nec auctor posuere,\n"
		"nulla tellus scelerisque metus, quis\n"
		"molestie mi dui id quam. Mauris\n"
		"vestibulum. Nam ullamcorper.\n");
	free (r);

	r = wordWrap (str, 80);
	test (r,
		"Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Integer metus pede,\n"
		"pretium vitae, rhoncus et, auctor sit amet, ligula. Integer volutpat orci et\n"
		"elit. Nunc tempus, urna at sollicitudin rutrum, arcu libero rhoncus lectus,\n"
		"vitae feugiat purus orci ultricies turpis. Pellentesque habitant morbi tristique\n"
		"senectus et netus et malesuada fames ac turpis egestas. Nam in pede. Etiam eu\n"
		"sem id urna ultricies congue. Vestibulum porttitor ultrices neque. Mauris\n"
		"semper, mauris ut feugiat ultricies, augue purus tincidunt elit, eu interdum\n"
		"ante nisl ac ante. Pellentesque dui. Vestibulum pretium, augue non cursus\n"
		"pretium, nibh dolor laoreet leo, sed pharetra pede libero non diam.\n"
		"\n"
		"Proin est nisi, gravida ac, vulputate id, fringilla sit amet, magna. Nam congue\n"
		"cursus magna. In malesuada, velit a gravida sodales, dolor nisl vestibulum orci,\n"
		"sit amet sagittis mauris tellus nec purus. Nulla eget risus. Quisque nec sapien\n"
		"blandit odio convallis ullamcorper. Lorem ipsum dolor sit amet, consectetuer\n"
		"adipiscing elit. Pellentesque cursus. Aliquam tempus neque vitae libero molestie\n"
		"ut auctor.\n"
		"\n"
		"In nec massa eu tortor vulputate suscipit. Nam tristique magna nec pede. Sed a\n"
		"nisi. Nulla sed augue ut risus placerat porttitor. Ut aliquam. Nulla facilisi.\n"
		"Nulla vehicula nibh ac sapien. Nunc facilisis dapibus ipsum. Donec sed mauris.\n"
		"Nulla quam nisi, laoreet non, dignissim posuere, lacinia nec, turpis. Mauris\n"
		"malesuada nisi sed enim. In hac habitasse platea dictumst. Fusce faucibus,\n"
		"turpis nec auctor posuere, nulla tellus scelerisque metus, quis molestie mi dui\n"
		"id quam. Mauris vestibulum. Nam ullamcorper.\n");
	free (r);

	return 0;
}
#endif /* __PROGTEST__ */

