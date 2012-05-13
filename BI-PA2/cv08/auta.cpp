#include <iostream>
#include "auta.h"
using namespace std;

int
main (int argc, char *argv[])
{
	Auto *pole[5] =
	{
		new Auto ("ABC", 1990),
		new Osobni ("DFE", 1991, 4),
		new Nakladni ("GHI", 1992, 200),
		new Osobni ("JKL", 1993, 5),
		new Auto ("MNO", 1994)
	};

	for (int i = 0; i < 5; i++)
	{
		cout << (i + 1) << ": ";
		pole[i]->print (cout);
		cout << endl;

		cout << "   SPZ: " << pole[i]->get_spz ()
		     << ", r.v.: " << pole[i]->get_rok_vyroby () << endl;

		delete pole[i];
	}

	return 0;
}

