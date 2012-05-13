#ifndef __PROGTEST__
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sstream>
using namespace std;

#include <cassert>
#endif /* ! __PROGTEST__ */

class CBigInt
{
	bool is_neg;

	unsigned int *parts;
	int n_parts;

	void extend (int index);
	unsigned int add_simple (int index, unsigned value);
	void add (int index, unsigned value);

public:
	CBigInt (int n = 0);
	CBigInt (const char *s);
	CBigInt (const CBigInt &bi);
	~CBigInt ();

	CBigInt &operator = (const CBigInt &bi);

	friend std::istream &operator >>
		(std::istream &is, CBigInt &bi);
	friend std::ostream &operator <<
		(std::ostream &os, const CBigInt &bi);

	CBigInt operator + (const CBigInt &bi) const;
	CBigInt &operator += (const CBigInt &bi);
	CBigInt operator * (const CBigInt &bi) const;
	CBigInt &operator *= (const CBigInt &bi);
};

CBigInt::CBigInt (int n)
{
	parts = new unsigned int[n_parts = 2];
	memset (parts, 0, sizeof *parts * n_parts);

	if (n < 0)
	{
		is_neg = true;
		parts[0] = -(long) n;
	}
	else
	{
		is_neg = false;
		parts[0] =  (long) n;
	}
}

CBigInt::CBigInt (const char *s)
{
	new (this) CBigInt ();

	if (*s == '-')
	{
		s++;
		is_neg = true;
	}
	else
		is_neg = false;

	int len = strlen (s);
	char *a = new char[len];

	for (int i = 0; i < len; i++)
		a[i] = s[i] - '0';

	unsigned int buf, bit_ctr, word_ctr;
	bool is_nonzero;

	buf = bit_ctr = word_ctr = 0;

	do
	{
		is_nonzero = false;

		unsigned int r = 0;
		for (int i = 0; i < len; i++)
		{
			r = r * 10 + a[i];
			a[i] = r >> 1;
			r &= 1;

			is_nonzero |= a[i] != 0;
		}

		buf |= (r << bit_ctr);
		if (++bit_ctr == sizeof buf * 8)
		{
			add (word_ctr++, buf);
			buf = bit_ctr = 0;
		}
	}
	while (is_nonzero);

	add (word_ctr, buf);
	delete [] a;
}

CBigInt::CBigInt (const CBigInt &bi)
{
	is_neg = bi.is_neg;
	parts = new unsigned int[n_parts = bi.n_parts];
	memcpy (parts, bi.parts, sizeof *parts * n_parts);
}

CBigInt::~CBigInt ()
{
	delete [] parts;
}

void
CBigInt::extend (int index)
{
	if (index >= n_parts)
	{
		int new_n = n_parts;
		do {
			new_n <<= 1;
		} while (index >= new_n);

		unsigned int *new_parts = new unsigned int[new_n];
		memcpy (new_parts, parts, sizeof *parts * n_parts);
		memset (new_parts + n_parts, 0,
			sizeof *parts * (new_n - n_parts));

		delete [] parts;
		n_parts = new_n;
		parts = new_parts;
	}
}

unsigned int
CBigInt::add_simple (int index, unsigned value)
{
	if (!value)
		return 0;

	extend (index);

	unsigned long long tmp =
		(unsigned long long) parts[index] + value;
	parts[index] = tmp;

	return tmp >> (sizeof *parts * 8);
}

void
CBigInt::add (int index, unsigned value)
{
	while (value)
		value = add_simple (index++, value);
}

CBigInt &
CBigInt::operator = (const CBigInt &bi)
{
	if (&bi != this)
	{
		this->~CBigInt ();
		new (this) CBigInt (bi);
	}
	return *this;
}

std::istream &
operator >> (std::istream &is, CBigInt &bi)
{
	std::istream::sentry s (is);
	if (!s)
		return is;

	string buf;
	int c;

	c = is.get ();
	if (is.eof ())
		return is;

	if (c == '-')
	{
		buf += c;

		c = is.get ();
		if (is.eof ())
			return is;
	}

	bool success = false;

	while (true)
	{
		if (c < '0' || c > '9')
		{
			is.unget ();
			break;
		}

		success = true;
		buf += c;

		c = is.get ();
		if (is.eof ())
			break;
	}

	if (success)
	{
		bi.~CBigInt ();
		new (&bi) CBigInt (buf.c_str ());
		is.clear ();
	}
	else
		is.setstate (ios::failbit);

	return is;
}

std::ostream &
operator << (std::ostream &os, const CBigInt &bi)
{
	CBigInt tmp (bi);
	bool is_nonzero = false;

	if (bi.is_neg)
	{
		for (int i = bi.n_parts; i--; )
			if (bi.parts[i])
				is_nonzero = true;
		if (is_nonzero)
			os << "-";
	}

	const int width = bi.n_parts * 10 + 4;
	char *buf = new char[width + 1];
	char *iter = buf + width;
	buf[width] = '\0';

	const unsigned long long int base
		= 1ULL << (sizeof *tmp.parts * 8);
	unsigned int all_in_one;

	do
	{
		all_in_one = 0;
		unsigned long long int r = 0;

		for (int i = tmp.n_parts; i--; )
		{
			r = tmp.parts[i] + r * base;
			tmp.parts[i] = r / 10000;
			r %= 10000;

			all_in_one |= tmp.parts[i];
		}

		iter[-1] = '0' +  (r         % 10);
		iter[-2] = '0' + ((r / 10)   % 10);
		iter[-3] = '0' + ((r / 100)  % 10);
		iter[-4] = '0' +  (r / 1000);

		iter -= 4;
	}
	while (all_in_one);

	while (iter[0] == '0' && iter[1])
		iter++;

	os << iter;
	delete [] buf;
	return os;
}

CBigInt
CBigInt::operator + (const CBigInt &bi) const
{
	CBigInt tmp (*this);
	tmp += bi;
	return tmp;
}

CBigInt &
CBigInt::operator += (const CBigInt &bi)
{
	if (&bi == this)
	{
		*this += CBigInt (bi);
		return *this;
	}

	if (!(is_neg ^ bi.is_neg))
	{
		for (int i = 0; i < bi.n_parts; i++)
			add (i, bi.parts[i]);
		return *this;
	}


	/* Make this number the same length. */
	if (n_parts < bi.n_parts)
		extend (bi.n_parts - 1);

	long long new_part = 0;

	/* Decrement. */
	for (int i = 0; i < bi.n_parts; i++)
	{
		parts[i] = new_part = new_part + parts[i] - bi.parts[i];
		new_part >>= sizeof (unsigned int) * 8;
	}
	for (int i = bi.n_parts; i < n_parts; i++)
	{
		parts[i] = new_part = new_part + parts[i];
		new_part >>= sizeof (unsigned int) * 8;
	}

	/* Fix the result eventually. */
	if (new_part)
	{
		is_neg = !is_neg;
		new_part = 1;

		for (int i = 0; i < n_parts; i++)
		{
			parts[i] = new_part = new_part + ~parts[i];
			new_part >>= sizeof (unsigned int) * 8;
		}
	}

	return *this;
}

CBigInt
CBigInt::operator * (const CBigInt &bi) const
{
	CBigInt tmp;
	int n_bi, n_this;

	for (n_bi = bi.n_parts; n_bi; n_bi--)
		if (bi.parts[n_bi - 1])
			break;
	for (n_this = n_parts; n_this; n_this--)
		if (parts[n_this - 1])
			break;

	int min_n = n_bi + n_this;
	int new_n = 4;
	while (min_n >= new_n)
		new_n <<= 1;

	delete [] tmp.parts;
	tmp.parts = new unsigned int[tmp.n_parts = new_n];
	memset (tmp.parts, 0, sizeof *tmp.parts * new_n);

	int i, k;
	unsigned long long p, x;

	for (i = 0; i < n_bi; i++)
	{
		p = 0;

		for (k = 0; k < n_this; k++)
		{
			p += (unsigned long long) bi.parts[i] * parts[k];
			x  = (unsigned long long) tmp.parts[i + k] + (unsigned int) p;
			tmp.parts[i + k] = x;
			p = (p >> sizeof *tmp.parts * 8) + (x >> sizeof *tmp.parts * 8);
		}
		tmp.parts[i + k] += p;
	}

	tmp.is_neg = is_neg ^ bi.is_neg;
	return tmp;
}

CBigInt &
CBigInt::operator *= (const CBigInt &bi)
{
	CBigInt tmp = *this * bi;

	this->~CBigInt ();
	new (this) CBigInt (tmp);

	return *this;
}

#ifndef __PROGTEST__
template<typename T> void
check_output (const T &x, const char *ref)
{
	ostringstream os;
	os << x;
	assert (os.str () == ref);
}

int
main (int argc, char *argv[])
{
	CBigInt a, b;
	istringstream is;

	a = 10;
	a += 20;
	check_output (a, "30");
	a *= 5;
	check_output (a, "150");

	b = a + 3;
	check_output (b, "153");
	b = a * 7;
	check_output (b, "1050");
	check_output (a, "150");


	a = 10;
	a += -20;
	check_output (a, "-10");
	a *= 5;
	check_output (a, "-50");

	b = a + 73;
	check_output (b, "23");
	b = a * -7;
	check_output (b, "350");
	check_output (a, "-50");


	a = "12345678901234567890";
	a += "-99999999999999999999";
	a *= "54321987654321987654";
	check_output (a, "-4761556948575111126880627366067073182286");


	a *= 0;
	check_output (a, "0");

	a = 10;
	b = a + "400";
	check_output (b, "410");
	b = a * "15";
	check_output (b, "150");
	check_output (a, "10");


	is.clear ();
	is.str (" 1234");
	is >> b;
	check_output (b, "1234");
	assert (is.fail () == 0);

	is.clear ();
	is.str (" 12 34");
	is >> b;
	check_output (b, "12");
	assert (is.fail () == 0);

	is.clear ();
	is.str ("999z");
	is >> b;
	check_output (b, "999");
	assert (is.fail () == 0);

	is.clear ();
	is.str ("abcd");
	is >> b;
	check_output (b, "999");
	assert (is.fail () == 1);

	a = "254914299662206004574928402102238739159025345046"
	    "49499674795728105506197351711176115173543125946";
	a += a; a *= a; a = a;
	check_output (a, "25992520068909184135253650275701889944"
		"693475232138916570714481105638788057107963148736622"
		"521965822302429599685064315149833118633496908472911"
		"91017805575928650918327261250324704572436873579664");

	/* Speed test: compute factorial. */
	a = 1;
	for (int i = 10000; i; i--)
		a *= i;
	cout << a << endl;

	return 0;
}
#endif /* ! __PROGTEST__ */

