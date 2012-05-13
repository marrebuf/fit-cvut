using namespace std;
#include <iostream>
#include <cstring>

// ===== Zamestnanec =========================================================
class Zamestnanec
{
protected:
	int _id;
	char _jmeno[25 + 1], _prijmeni[36 + 1];
public:
	Zamestnanec (int id, const char *jmeno, const char *prijmeni);
	virtual int plat () const = 0;
	virtual void vypis () const = 0;
};

Zamestnanec::Zamestnanec (int id,
	const char *jmeno, const char *prijmeni)
{
	_jmeno[25] = _prijmeni[36] = '\0';

	_id = id;
	strncpy (_jmeno, jmeno, 25);
	strncpy (_prijmeni, prijmeni, 36);
}

// ===== Delnik ==============================================================
class Delnik : public Zamestnanec
{
	int _mzda, _hodiny;
public:
	Delnik (int id, const char *jmeno, const char *prijmeni,
		int mzda, int hodiny) : Zamestnanec (id, jmeno, prijmeni),
		_mzda (mzda), _hodiny (hodiny) {}
	virtual int plat () const;
	virtual void vypis () const;
};

int
Delnik::plat () const
{
	return _mzda * _hodiny;
}

void
Delnik::vypis () const
{
	cout << "Dělník #" << _id << ": " << _jmeno << " " << _prijmeni
	     << " (plat " << plat () << " mincí)" << endl;
}

// ===== Urednik =============================================================
class Urednik : public Zamestnanec
{
	int _plat;
public:
	Urednik (int id, const char *jmeno, const char *prijmeni,
		int plat) : Zamestnanec (id, jmeno, prijmeni),
		_plat (plat) {}
	virtual int plat () const;
	virtual void vypis () const;
};

int
Urednik::plat () const
{
	return _plat;
}

void
Urednik::vypis () const
{
	cout << "Úředník #" << _id << ": " << _jmeno << " " << _prijmeni
	     << " (plat " << plat () << " mincí)" << endl;
}

// ===== Firma ===============================================================
class Firma
{
	int _max;
	const Zamestnanec *_lidi[32];
	int _lidi_ted;
public:
	Firma ();
	int vloz (const Zamestnanec &zam);
	void odeber (const Zamestnanec &zam);
	void vypis () const;
	void platy () const;
};

Firma::Firma ()
{
	_max = sizeof _lidi / sizeof *_lidi;
	_lidi_ted = 0;
}

int
Firma::vloz (const Zamestnanec &zam)
{
	if (_lidi_ted == _max)
		return 0;

	_lidi[_lidi_ted++] = &zam;
	return 1;
}

void
Firma::odeber (const Zamestnanec &zam)
{
	for (int i = 0; i < _lidi_ted; i++)
		if (_lidi[i] == &zam)
			_lidi[i] = _lidi[--_lidi_ted];
}

void
Firma::vypis () const
{
	cout << "Firma se skládá z:" << endl;
	for (int i = 0; i < _lidi_ted; i++)
	{
		cout << "  ";
		_lidi[i]->vypis ();
	}
}

void
Firma::platy () const
{
	int suma = 0;
	for (int i = 0; i < _lidi_ted; i++)
		suma += _lidi[i]->plat ();
	cout << "Všichni dohromady si vydělají " << suma
	     << " čokoládových mincí" << endl;
}

// ===== Main ================================================================
int
main (int argc, char *argv[])
{
	Delnik delnici[] =
	{
		Delnik (1, "Pavel", "X", 100, 120),
		Delnik (2, "Ondra", "Y", 200, 110)
	};
	Urednik urednici[] =
	{
		Urednik (3, "Jan",     "A", 15000),
		Urednik (4, "Bohumil", "B", 13200)
	};

	Firma f;
	for (unsigned i = 0; i < sizeof delnici  / sizeof *delnici;  i++)
		f.vloz (delnici [i]);
	for (unsigned i = 0; i < sizeof urednici / sizeof *urednici; i++)
		f.vloz (urednici[i]);

	f.vypis ();
	f.platy ();

	cout << "... snížila se dostupnost čokolády ..." << endl;
	cout << "... musíme propouštět ..." << endl;
	f.odeber (delnici[0]);

	f.vypis ();
	f.platy ();

	cout << "... krize zažehnána ..." << endl;
	Delnik franta (5, "František", "Z", 120, 130);
	f.vloz (franta);

	f.vypis ();
	f.platy ();

	return 0;
}

