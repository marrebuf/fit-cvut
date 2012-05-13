#include <iostream>

using namespace std;

int ctiPole (int &delka, int *&pole)
{
	cout << "Napiš počet prvků:" << endl;
	cin >> delka;
	if (!cin.good ())
		return 0;

	pole = new int[delka];

	cout << "Zadej prvky:" << endl;
	for (int i = 0; i < delka; i++)
	{
		cin >> pole[i];
		if (!cin.good ())
			return 0;
	}

	return 1;
}

void vypisPole (int delka, int *pole)
{
	cout << "Pole:" << endl;
	for (int i = 0; i < delka; i++)
		cout << i << ": " << pole[i] << endl;
}

int main ()
{
	int delka, *pole;

	if (ctiPole (delka, pole))
	{
		vypisPole (delka, pole);
		delete [] pole;
	}
	else
		cerr << "Nepovedlo se načíst pole." << endl;
}

