#ifndef __PROGTEST__
#include <iostream>
#include <iomanip>
#include <string>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>
using namespace std;
#else /* __ PROGTEST__ */
#define assert(expr)
#endif /* __PROGTEST__ */

/* Why repeat it over and over again? */
#define RecordID const string &name, const string &address
#define OutRecordID string &name, string &address

class CPhoneBook
{
	template<typename T> bool
	static bsearch (const T *key, T *base, size_t nmemb,
		T *&pos, int (*cmp) (const T *key, const T *item));

	/* Sorted table of people. */
	struct Record
	{
		string *name, *address;
		int phone_id, n_phones;

		Record (string *name = NULL, string *address = NULL,
			int phone_id = -1, int n_phones = 0)
		{
			this->name     = name;
			this->address  = address;
			this->phone_id = phone_id;
			this->n_phones = n_phones;
		}

		~Record ()
		{
			if (name)     delete name;
			if (address)  delete address;
		}
	};

	Record *records;
	int r_size, r_len;

	bool get_record (RecordID, Record *&pos) const;
	static int compare_record (const Record *key, const Record *item);
	void destroy_records ();

	/* Sorted table of phone numbers. */
	struct Phone
	{
		string *number;
		int record_id, next_id;

		Phone (string *number = NULL,
			int record_id = -1, int next_id = -1)
		{
			this->number    = number;
			this->record_id = record_id;
			this->next_id   = next_id;
		}

		~Phone ()
		{
			if (number)   delete number;
		}
	};

	Phone *phones;
	int p_size, p_len;

	bool get_phone (const string &number, Phone *&pos) const;
	static int compare_phone (const Phone *key, const Phone *item);
	void destroy_phones ();

	bool add_phone (const string &number, int &idx);

	void remove_phone (Phone *ph);
	void remove_record (Record *rec);

public:
    static const int MAX_PHONES_PER_USER = 16;

	CPhoneBook ();
	~CPhoneBook ();

	bool Add (RecordID, const string &phone);

	bool Del (RecordID);
	bool Del (const string &phone);

	bool Search (RecordID, int idx, string &phone) const;
	bool Search (const string &phone, OutRecordID) const;

#ifndef __PROGTEST__
	void print ();
#endif /* ! __PROGTEST__ */
};


/* ===== Private methods ================================================== */

template<typename T> bool
CPhoneBook::bsearch (const T *key, T *base, size_t nmemb,
	T *&pos, int (*cmp) (const T *key, const T *item))
{
	int min = 0, max = nmemb - 1;

	while (max >= min)
	{
		int mid = (min + max) >> 1;
		int n = cmp (key, base + mid);

		if      (n < 0)
			min = mid + 1;
		else if (n > 0)
			max = mid - 1;
		else
		{
			pos = base + mid;
			return true;
		}
	}

	pos = base + min;
	return false;
}

bool
CPhoneBook::get_record (RecordID, Record *&pos) const
{
	Record key (new string (name), new string (address));
	return bsearch (&key, records, r_len, pos, compare_record);
}

int
CPhoneBook::compare_record (const Record *key, const Record *item)
{
	int name_cmp = strcmp (item->name->c_str (), key->name->c_str ());
	if (name_cmp)  return name_cmp;

	return strcmp (item->address->c_str (), key->address->c_str ());
}

void
CPhoneBook::destroy_records ()
{
	for (int i = 0; i < r_len; i++)
		records[i].~Record ();
	free (records);
}

bool
CPhoneBook::get_phone (const string &number, Phone *&pos) const
{
	Phone key (new string (number));
	return bsearch (&key, phones, p_len, pos, compare_phone);
}

int
CPhoneBook::compare_phone (const Phone *key, const Phone *item)
{
	return strcmp (item->number->c_str (), key->number->c_str ());
}

void
CPhoneBook::destroy_phones ()
{
	for (int i = 0; i < p_len; i++)
		phones [i].~Phone  ();
	free (phones);
}

void
CPhoneBook::remove_record (Record *rec)
{
	int index = rec - records;

	assert (index < r_len);

	r_len--;
	rec->~Record ();

	memmove (rec, rec + 1,
		sizeof *records * (r_len - index));

	/* Update indexes in `phones'. */
	for (int i = 0; i < p_len; i++)
		if (phones[i].record_id > index)
			phones[i].record_id--;
}

void
CPhoneBook::remove_phone (Phone *ph)
{
	int index = ph - phones;

	assert (index < p_len);

	p_len--;
	ph->~Phone ();

	memmove (ph, ph + 1,
		sizeof *phones * (p_len - index));

	/* Update indexes in `phones'. */
	for (int i = 0; i < p_len; i++)
		if (phones[i].next_id > index)
			phones[i].next_id--;
	/* Update indexes in `records'. */
	for (int i = 0; i < r_len; i++)
		if (records[i].phone_id > index)
			records[i].phone_id--;
}

bool
CPhoneBook::add_phone (const string &number, int &idx)
{
	Phone *ph;
	if (get_phone (number, ph))
		return false;

	int index = ph - phones;

	if (p_len < p_size)
		memmove (ph + 1, ph,
			sizeof *ph * (p_len - index));
	else
	{
		p_size <<= 1;
		Phone *new_phones = reinterpret_cast<Phone *>
			(malloc (p_size * sizeof *new_phones));

		memcpy (new_phones, phones,
			sizeof *phones * index);
		memcpy (new_phones + index + 1, phones + index,
			sizeof *phones * (p_len - index));

		free (phones);
		phones = new_phones;
	}

	new (phones + index) Phone (new string (number));
	p_len++;

	/* Update indexes in `phones'. */
	for (int i = 0; i < p_len; i++)
		if (phones[i].next_id >= index)
			phones[i].next_id++;
	/* Update indexes in `records'. */
	for (int i = 0; i < r_len; i++)
		if (records[i].phone_id >= index)
			records[i].phone_id++;

	idx = index;
	return true;
}


/* ===== Public API ======================================================= */

CPhoneBook::CPhoneBook ()
{
	r_len = 0;
	r_size = 1024;
	records = reinterpret_cast<Record *>
		(malloc (r_size * sizeof *records));

	p_len = 0;
	p_size = 1024;
	phones = reinterpret_cast<Phone *>
		(malloc (p_size * sizeof *phones));
}

CPhoneBook::~CPhoneBook ()
{
	destroy_records ();
	destroy_phones ();
}

bool
CPhoneBook::Add (RecordID, const string &phone)
{
	Record *rec;
	int ph_index;

	if (get_record (name, address, rec))
	{
		if (rec->n_phones == MAX_PHONES_PER_USER)
			return false;

		if (!add_phone (phone, ph_index))
			return false;

#ifdef PREPEND
		/* Prepend the phone to the list. */
		phones[ph_index].record_id = rec - records;
		phones[ph_index].next_id = rec->phone_id;
		rec->phone_id = ph_index;
		rec->n_phones++;
#else /* ! PREPEND */
		/* Append the phone to the list. */
		phones[ph_index].record_id = rec - records;
		int last_phone = rec->phone_id;
		for (int ctr = rec->n_phones++; --ctr; )
			last_phone = phones[last_phone].next_id;
		phones[last_phone].next_id = ph_index;
#endif /* ! PREPEND */

		return true;
	}

	if (!add_phone (phone, ph_index))
		return false;

	int index = rec - records;

	if (r_len < r_size)
		memmove (rec + 1, rec,
			sizeof *rec * (r_len - index));
	else
	{
		r_size <<= 1;
		Record *new_records = reinterpret_cast<Record *>
			(malloc (r_size * sizeof *new_records));

		memcpy (new_records, records,
			sizeof *records * index);
		memcpy (new_records + index + 1, records + index,
			sizeof *records * (r_len - index));

		free (records);
		records = new_records;
	}

	new (records + index) Record (new string (name),
		new string (address), ph_index, 1);
	r_len++;

	/* Update indexes in `phones'. */
	for (int i = 0; i < p_len; i++)
		if (phones[i].record_id >= index)
			phones[i].record_id++;

	phones[ph_index].record_id = index;
	return true;
}

bool
CPhoneBook::Del (RecordID)
{
	Record *rec;
	if (!get_record (name, address, rec))
		return false;

	/* Remove all associated phone records. */
	while (rec->phone_id != -1)
	{
		Phone *ph = &phones[rec->phone_id];
		rec->phone_id = phones[rec->phone_id].next_id;
		remove_phone (ph);
	}

	remove_record (rec);
	return true;
}

bool
CPhoneBook::Del (const string &phone)
{
	Phone *ph;
	if (!get_phone (phone, ph))
		return false;

	assert (ph->record_id != -1);
	Record *rec = &records[ph->record_id];

	/* Remove the whole record if needed. */
	if (!--rec->n_phones)
	{
		remove_phone (ph);
		remove_record (rec);
		return true;
	}

	/* Or just unlink the phone from the list. */
	Phone *iter = &phones[rec->phone_id];
	int *ref = &rec->phone_id;

	while (true)
	{
		if (iter == ph)
			break;

		assert (iter->next_id != -1);
		ref = &iter->next_id;
		iter = &phones[iter->next_id];
	}

	*ref = iter->next_id;
	remove_phone (iter);
	return true;
}

bool
CPhoneBook::Search (const string &phone, OutRecordID) const
{
	Phone *ph;
	if (!get_phone (phone, ph))
		return false;

	assert (ph->record_id != -1);
	Record *rec = &records[ph->record_id];

	name    = *rec->name;
	address = *rec->address;
	return true;
}

bool
CPhoneBook::Search (RecordID, int idx, string &phone) const
{
	Record *rec;
	if (!get_record (name, address, rec))
		return false;

	assert (rec->phone_id != -1);
	Phone *ph = &phones[rec->phone_id];

	while (idx--)
	{
		if (ph->next_id == -1)
			return false;
		ph = &phones[ph->next_id];
	}

	phone = *ph->number;
	return true;
}

#ifndef __PROGTEST__
void
CPhoneBook::print ()
{
	cout << "  PhoneBook" << endl;
	for (int i = 0; i < r_len; i++)
	{
		cout << "    '" << *records[i].name
		     << "' '"   << *records[i].address
		     << "' (";

		for (int pid = records[i].phone_id; ; )
		{
			cout << *phones[pid].number;

			pid = phones[pid].next_id;
			if (pid == -1)
				break;
			cout << " ";
		}

		cout << ")" << endl;
	}
}
#endif /* ! __PROGTEST__ */


/* ===== Tests ============================================================ */

#ifndef __PROGTEST__
int
main (int argc, char *argv[])
{
	string phone, name, addr;

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
	assert (b1.Search ("Brown", "Second street", 0, phone) == true);
	assert (phone == "7654321");
	assert (b1.Search ("Brown", "Second street", 1, phone) == false);
	assert (b1.Search ("Hacker", "Oak road", 0, phone) == false);
	assert (b1.Search ("Smith", "Oak road", 0, phone) == true);
	assert (phone == "123456");
	assert (b1.Search ("Smith", "Oak road", 1, phone) == false);
	assert (b1.Del ("Smith", "Oak road") == true);
	b1.print ();
	assert (b1.Search ("Smith", "Oak road", 0, phone) == false);

	CPhoneBook b2;
	cout << ">>> Book b2" << endl;

	assert (b2.Add ("Smith", "Michigan avenue", "9123456") == true);
	b2.print ();
	assert (b2.Search ("Smith", "Michigan avenue", 0, phone) == true);
	assert (phone == "9123456");
	assert (b2.Search ("Smith", "Michigan avenue", 1, phone) == false);
	assert (b2.Del ("Smith", "Michigan avenue") == true);
	b2.print ();
	assert (b2.Search ("Smith", "Michigan avenue", 0, phone) == false);
	assert (b2.Del ("Smith", "Michigan avenue") == false);
	assert (b2.Add ("Smith", "Michigan avenue", "9123456") == true);
	b2.print ();
	assert (b2.Search ("Smith", "Michigan avenue", 0, phone) == true);
	assert (phone == "9123456");
	assert (b2.Search ("Smith", "Michigan avenue", 1, phone) == false);
	assert (b2.Add ("Smith", "Michigan avenue", "9123456") == false);
	assert (b2.Add ("Smith", "Michigan avenue", "10293847") == true);
	b2.print ();
	assert (b2.Del ("Smith", "Michigan avenue") == true);
	b2.print ();
	assert (b2.Search ("Smith", "Michigan avenue", 0, phone) == false);

	CPhoneBook b3;
	cout << ">>> Book b3" << endl;

	assert (b3.Add ("Smith", "Michigan avenue", "1234") == true);
	b3.print ();
	assert (b3.Add ("Smith", "Michigan avenue", "2233") == true);
	b3.print ();
	assert (b3.Add ("Smith", "Michigan avenue", "3344") == true);
	b3.print ();
	assert (b3.Add ("Brown", "Oak road", "5678") == true);
	b3.print ();
	assert (b3.Add ("Brown", "Oak road", "6677") == true);
	b3.print ();
	assert (b3.Search ("Smith", "Michigan avenue", 0, phone) == true);
	assert (phone == "1234");
	assert (b3.Search ("Smith", "Michigan avenue", 1, phone) == true);
	assert (phone == "2233");
	assert (b3.Search ("Smith", "Michigan avenue", 2, phone) == true);
	assert (phone == "3344");
	assert (b3.Search ("Smith", "Michigan avenue", 3, phone) == false);
	assert (b3.Search ("Brown", "Oak road", 0, phone) == true);
	assert (phone == "5678");
	assert (b3.Search ("Brown", "Oak road", 1, phone) == true);
	assert (phone == "6677");
	assert (b3.Search ("Brown", "Oak road", 2, phone) == false);
	assert (b3.Search ("2233", name, addr) == true);
	assert (name == "Smith");
	assert (addr == "Michigan avenue");
	assert (b3.Del ("6677") == true);
	b3.print ();
	assert (b3.Search ("Brown", "Oak road", 0, phone) == true);
	assert (phone == "5678");
	assert (b3.Search ("Brown", "Oak road", 1, phone) == false);
	assert (b3.Del ("5678") == true);
	b3.print ();
	assert (b3.Search ("Brown", "Oak road", 0, phone) == false);

	CPhoneBook b4;

	static const char *first_names[9] = {"Marián", "Jan", "Ježíš",
		"Jiří", "Pinkie", "Slávek", "Jakub", "Ernest", "Ludovít"};
	static const char *last_names[9] = {"Bílý", "Černý", "Pie",
		"Mrtvý", "John", "Vagner", "Šebek", "Strnad", "Lakatoš"};
	static const char *ad_start[9] = {"Oranžová", "Zelená", "Zlatá",
		"Černá", "Svatá", "Nekonečná", "Zlá", "Úchylná", "Moje"};
	static const char *ad_end[9] = {"Lhota", "Lhotka", "Vesnička",
		"Brána", "Propadlina", "Holka", "Díra", "Hora", "Věc"};

	#define REMEMBERED 128
	#define PHONE_MAX 8192
	string names[REMEMBERED], addresses[REMEMBERED], phones[REMEMBERED];

	cout << ">>> Book b4" << endl;
	cout << "  Adding random items" << endl;
	for (int i = 0; i < REMEMBERED; i++)
	{
		int r1 = rand () % 9,
		    r2 = rand () % 9,
		    r3 = rand () % 9,
			r4 = rand () % 9,
			r5 = rand () % PHONE_MAX;

		char buf[12];
		snprintf (buf, sizeof buf, "%d", r5);

		names[i]     = string (first_names[r1]) + " " + last_names[r2];
		addresses[i] = string (ad_start[r3]) + " " + ad_end[r4];
		phones[i]    = buf;

		if (!b4.Add (names[i], addresses[i], phones[i]))
			names[i] = addresses[i] = phones[i] = "N/A";
	}

	cout << "  Searching for the phones" << endl;
	for (int i = 0; i < REMEMBERED; i++)
	{
		string name, address;

		if (b4.Search (phones[i], name, address))
		{
			if (name != names[i])
				cout << "    Mismatch at #" << i << " for '" << phones[i]
				     << "': " << "'" << name << "' should be '"
				     << names[i] << "'" << endl;
		}
	}

	cout << "  Searching for random people" << endl;
	for (int i = 0; i < 32768; i++)
	{
		string phone;
		int r1 = rand () % 9,
		    r2 = rand () % 9,
		    r3 = rand () % 9,
			r4 = rand () % 9,
			r5 = rand () % 9;

		b4.Search (string (first_names[r1]) + " " + last_names[r2],
			string (ad_start[r3]) + " " + ad_end[r4], r5, phone);
	}

	cout << "  Removing random items" << endl;
	for (int i = 0; i < 32768; i++)
	{
		int r1 = rand () % 9,
		    r2 = rand () % 9,
		    r3 = rand () % 9,
			r4 = rand () % 9;

		b4.Del (string (first_names[r1]) + " " + last_names[r2],
			string (ad_start[r3]) + " " + ad_end[r4]);
	}

	return 0;
}
#endif /* ! __PROGTEST__ */

