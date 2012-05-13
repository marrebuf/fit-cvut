#include <iostream>
using namespace std;

/* ===== Class Bag ======================================================= */

class Bag
{
	int size, len, *a;
	int find (int value) const;

public:
	Bag (int size);
	Bag (const Bag &b);
	~Bag ();

	void vloz (int value);
	bool odeber (int value);
	bool jeTam (int value) const;

	Bag &operator += (int value);
	Bag &operator = (const Bag &b);
	friend ostream &operator << (ostream &s, const Bag &b);
};

Bag::Bag (int size = 10)
{
	a = new int[size];
	this->size = size;
	len = 0;
}

Bag::Bag (const Bag &b)
{
	a = new int[b.size];
	for (int i = 0; i < b.len; i++)
		a[i] = b.a[i];

	size = b.size;
	len = b.len;
}

Bag::~Bag ()
{
	delete [] a;
}

void
Bag::vloz (int value)
{
	if (len == size)
	{
		int *n = new int[size << 1];
		for (int i = 0; i < size; i++)
			n[i] = a[i];

		a = n;
		size <<= 1;
	}

	a[len++] = value;
}

bool
Bag::odeber (int value)
{
	for (int i = 0; i < len; i++)
	{
		if (a[i] == value)
		{
			a[i] = a[--len];
			return true;
		}
	}
	return false;
}

bool
Bag::jeTam (int value) const
{
	for (int i = 0; i < len; i++)
		if (a[i] == value)
			return true;
	return false;
}

int
Bag::find (int value) const
{
	for (int i = 0; i < len; i++)
	{
		if (a[i] == value)
			return i;
	}
	return -1;
}

Bag &
Bag::operator += (int value)
{
	vloz (value);
	return *this;
}

Bag &
Bag::operator = (const Bag &b)
{
	if (this == &b)
		return *this;

	delete [] a;

	a = new int [b.size];
	for (int i = 0; i < b.len; i++)
		a[i] = b.a[i];

	size = b.size;
	len = b.len;
	return *this;
}

ostream &
operator << (ostream &s, const Bag &b)
{
	int i;

	for (i = 0; i < b.len - 1; i++)
		s << b.a[i] << " ";
	s << b.a[i];
	return s;
}


/* ===== Main ============================================================ */

int
main ()
{
	Bag b1;
	b1.vloz (1);
	b1.vloz (3);
	b1.vloz (1);
	b1.vloz (2);
	cout << "b1 = " << b1 << endl;

	Bag b2 = b1;
	b1.odeber (1);
	cout << "b1 = " << b1 << endl;
	cout << "b2 = " << b2 << endl;

	Bag b3;
	b3 = b1;
	b1.odeber (1);
	cout << "b1 = " << b1 << endl;
	cout << "b3 = " << b3 << endl;

	return 0;
}

