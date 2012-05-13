#include <iostream>
#include <string>
#include <cassert>

#include "array.h"
#include "queue-list.h"
#include "queue-array.h"

int
main (int argc, char *argv[])
{
	QueueList<int> ql;

	assert (ql.empty () == true);
	ql.add (10).add (20);
	assert (ql.empty () == false);
	assert (ql.front () == 10);
	ql.remove ();
	assert (ql.front () == 20);
	ql.remove ();
	assert (ql.empty () == true);

	for (int i = 0; i < 100; i++)
		ql.add (i);
	for (int i = 0; i < 100; i++)
	{
		assert (ql.front () == i);
		ql.remove ();
	}
	assert (ql.empty () == true);

	QueueArray<int> qa;

	assert (qa.empty () == true);
	qa.add (10).add (20);
	assert (qa.empty () == false);
	assert (qa.front () == 10);
	qa.remove ();
	assert (qa.front () == 20);
	qa.remove ();
	assert (qa.empty () == true);

	for (int i = 0; i < 100; i++)
		qa.add (i);
	for (int i = 0; i < 100; i++)
	{
		assert (qa.front () == i);
		qa.remove ();
	}
	assert (qa.empty () == true);

	return 0;
}

