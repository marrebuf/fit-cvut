#ifndef __PROGTEST__
#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
using namespace std;
#endif /* __PROGTEST__ */

/* Why repeat it over and over again? */
#define PhoneID const string &name, const string &address

class CPhoneBook
{
	int size, len;
	struct ID
	{
		string name, address, phone;
		ID &operator = (const ID &id)
		{
			name = id.name;
			address = id.address;
			phone = id.phone;
			return *this;
		}
	}
	*items;

	bool get_item(PhoneID, CPhoneBook::ID *&position) const;
	static int compare_id (const ID *key, const ID *item);

public:
	CPhoneBook (void);
	~CPhoneBook (void);

	bool Add (PhoneID, const string &phone);
	bool Del (PhoneID);
	bool Search (PhoneID, string &phone) const;

#ifndef __PROGTEST__
	void print ();
#endif /* ! __PROGTEST__ */
};


/* ===== Private methods ================================================== */

int
CPhoneBook::compare_id (const ID *key, const ID *item)
{
	int name_cmp = strcmp (item->name.c_str (), key->name.c_str ());
	if (name_cmp)  return name_cmp;

	return strcmp (item->address.c_str (), key->address.c_str ());
}

bool
CPhoneBook::get_item (PhoneID, CPhoneBook::ID *&pos) const
{
	ID key = {name, address, ""};
	int min = 0, max = len - 1;

	while (max >= min)
	{
		int mid = (min + max) >> 1;
		int cmp = compare_id (&key, items + mid);

		if      (cmp < 0)
			min = mid + 1;
		else if (cmp > 0)
			max = mid - 1;
		else
		{
			pos = items + mid;
			return true;
		}
	}

	pos = items + min;
	return false;
}


/* ===== Public API ======================================================= */

CPhoneBook::CPhoneBook (void)
{
	len = 0;
	size = 1024;
	items = reinterpret_cast<ID *>
		(malloc (size * sizeof *items));
}

CPhoneBook::~CPhoneBook (void)
{
	int i;

	for (i = 0; i < len; i++)
		items[i].~ID ();
	free (items);
}

bool
CPhoneBook::Add (PhoneID, const string &phone)
{
	ID *pos, *iter;
	if (get_item (name, address, pos))
		return false;

	if (len < size)
	{
		/* Construct a single new ID object. */
		iter = items + len++;
		new (iter) ID ();
		/* Move the tail. */
		for (; iter > pos; iter--)
			iter[0] = iter[-1];
		/* Insert at the new position. */
		iter->name = name;
		iter->address = address;
		iter->phone = phone;

		return true;
	}

	/* The less common case: reallocate. */
	size <<= 1;
	ID *new_items = reinterpret_cast<ID *>
		(malloc (size * sizeof *new_items));

	/* Construct ID objects in the new array. */
	for (iter = new_items + len; iter >= new_items; iter--)
		new (iter) ID ();

	int i;

	/* Copy the head. */
	for (i = 0; i < pos - items; i++)
		new_items[i] = items[i];

	/* Insert at the new position. */
	new_items[i].name = name;
	new_items[i].address = address;
	new_items[i].phone = phone;

	/* Copy the tail. */
	for (; i < len; i++)
		new_items[i + 1] = items[i];

	/* Use the new array from now. */
	this->~CPhoneBook ();
	items = new_items;
	len++;
	return true;
}

bool
CPhoneBook::Del (PhoneID)
{
	ID *item;
	if (!get_item (name, address, item))
		return false;

	ID *end = items + --len;
	for (; item < end; item++)
		item[0] = item[1];
	item->~ID ();
	return true;
}

bool
CPhoneBook::Search (PhoneID, string &phone) const
{
	ID *item;
	if (!get_item (name, address, item))
		return false;

	phone = item->phone;
	return true;
}

#ifndef __PROGTEST__
void
CPhoneBook::print ()
{
	cout << "  PhoneBook" << endl;
	for (int i = 0; i < len; i++)
		cout << "    '" << items[i].name
		     << "' '"   << items[i].address
		     << "' '"   << items[i].phone << "'" << endl;
}
#endif /* ! __PROGTEST__ */


/* ===== Tests ============================================================ */

#ifndef __PROGTEST__
#include <cassert>
#include <ctime>

int
main (int argc, char *argv[])
{
	string phone;

	CPhoneBook b1;

	cout << ">>> Book b1" << endl;
	assert (b1.Add ("Smith", "Oak road", "123456") == true);
	b1.print ();
	assert (b1.Add ("Brown", "Second street", "7654321") == true);
	b1.print ();
	assert (b1.Add ("Hacker", "5-th avenue", "02348") == true);
	b1.print ();
	assert (b1.Add ("Hacker", "7-th avenue", "13278") == true);
	b1.print ();
	assert (b1.Search ("Brown", "Second street", phone) == true);
	assert (phone == "7654321");
	assert (b1.Search ("Hacker", "Oak road", phone) == false);
	assert (b1.Search ("Smith", "Oak road", phone) == true);
	assert (phone == "123456");
	assert (b1.Del ("Smith", "Oak road") == true);
	b1.print ();
	assert (b1.Search ("Smith", "Oak road", phone) == false);

	CPhoneBook b2;

	cout << ">>> Book b2" << endl;
	assert (b2.Add ("Smith", "Michigan avenue", "9123456") == true);
	b2.print ();
	assert (b2.Search ("Smith", "Michigan avenue", phone) == true);
	assert (phone == "9123456");
	assert (b2.Del ("Smith", "Michigan avenue") == true);
	b2.print ();
	assert (b2.Search ("Smith", "Michigan avenue", phone) == false);
	assert (b2.Del ("Smith", "Michigan avenue") == false);
	assert (b2.Add ("Smith", "Michigan avenue", "9123456") == true);
	b2.print ();
	assert (b2.Search ("Smith", "Michigan avenue", phone) == true);
	assert (phone == "9123456");
	assert (b2.Add ("Smith", "Michigan avenue", "9123456") == false);
	assert (b2.Add ("Smith", "Michigan avenue", "10293847") == false);
	assert (b2.Del ("Smith", "Michigan avenue") == true);
	b2.print ();
	assert (b2.Search ("Smith", "Michigan avenue", phone) == false);

	CPhoneBook b3;

	static const char *first_names[9] = {"Marián", "Jan", "Ježíš",
		"Jiří", "Pinkie", "Slávek", "Jakub", "Ernest", "Ludovít"};
	static const char *last_names[9] = {"Bílý", "Černý", "Pie",
		"Mrtvý", "John", "Vagner", "Šebek", "Strnad", "Lakatoš"};
	static const char *numbers[9] = {"112567", "123456", "012012",
		"1188", "020202", "210984", "109852", "203984", "158"};
	static const char *ad_start[9] = {"Oranžová", "Zelená", "Zlatá",
		"Černá", "Svatá", "Nekonečná", "Zlá", "Úchylná", "Moje"};
	static const char *ad_end[9] = {"Lhota", "Lhotka", "Vesnička",
		"Brána", "Propadlina", "Holka", "Díra", "Hora", "Věc"};

	cout << ">>> Book b3" << endl;
	cout << "  Adding random items" << endl;
	for (int i = 0; i < 16384; i++)
	{
		int r1 = rand () % 9,
		    r2 = rand () % 9,
		    r3 = rand () % 9,
		    r4 = rand () % 9,
		    r5 = rand () % 9;

		b3.Add (string (first_names[r1]) + " " + last_names[r2],
			string (ad_start[r3]) + " " + ad_end[r4],
			string (numbers[r5]));
	}

	cout << "  Removing random items" << endl;
	for (int i = 0; i < 16384; i++)
	{
		int r1 = rand () % 9,
		    r2 = rand () % 9,
		    r3 = rand () % 9,
		    r4 = rand () % 9;

		b3.Del (string (first_names[r1]) + " " + last_names[r2],
			string (ad_start[r3]) + " " + ad_end[r4]);
	}

	return 0;
}
#endif /* ! __PROGTEST__ */

