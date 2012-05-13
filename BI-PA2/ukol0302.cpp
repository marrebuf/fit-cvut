#ifndef __PROGTEST__
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sstream>
using namespace std;

#include <cassert>
#endif /* ! __PROGTEST__ */

class CDateTime
{
	long long int stamp;
	static const int m_days[];

	static inline bool is_leap (int y)
	{
		return (y % 4 == 0) &&
			((y % 100 != 0) || (y % 400 == 0));
	}

	void decompose (int &y, int &m, int &d,
		int &h, int &min, int &s) const;

public:
	CDateTime (int y = 2000, int m = 1, int d = 1,
		int h = 0, int min = 0, int s = 0);
	CDateTime &operator = (const char *date);

	long long int operator - (const CDateTime &date) const;
	CDateTime operator + (int s) const;

	bool operator == (const CDateTime &date) const;
	bool operator != (const CDateTime &date) const;
	bool operator <= (const CDateTime &date) const;
	bool operator >= (const CDateTime &date) const;
	bool operator <  (const CDateTime &date) const;
	bool operator >  (const CDateTime &date) const;

	int operator [] (const char *part) const;

	friend std::istream &operator >>
		(std::istream &s, CDateTime &date);
	friend std::ostream &operator <<
		(std::ostream &s, const CDateTime &date);
};

const int CDateTime::m_days[] =
		{0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

#define SEC_IN_DAY  86400

void
CDateTime::decompose (int &y, int &m, int &d,
	int &h, int &min, int &s) const
{
	long long int rem = this->stamp;

	for (y = 1600; ; y++)
	{
		int y_secs = SEC_IN_DAY * (365 + is_leap (y));
		if (rem >= y_secs)
			rem -= y_secs;
		else
			break;
	}

	for (m = 1; ; m++)
	{
		int m_secs = SEC_IN_DAY * (m_days[m]
			+ (m == 2 && is_leap (y)));
		if (rem >= m_secs)
			rem -= m_secs;
		else
			break;
	}

	d = 1 + rem / SEC_IN_DAY;
	rem %= SEC_IN_DAY;

	h = rem / 3600;
	rem %= 3600;
	min = rem / 60;
	rem %= 60;
	s = rem;
}

CDateTime::CDateTime (int y, int m, int d,
	int h, int min, int s)
{
	stamp = d - 1 + (m > 2 && is_leap (y));
	while (--y >= 1600)
		stamp += 365 + is_leap (y);
	while (--m >  0)
		stamp += m_days[m];

	stamp *= SEC_IN_DAY;
	stamp += s + 60 * (min + 60 * h);
}

CDateTime &
CDateTime::operator = (const char *date)
{
	int y, m, d, h, min, s;
	if (sscanf (date, "%04d-%02d-%02d %02d:%02d:%02d",
		&y, &m, &d, &h, &min, &s) == 6)
		new (this) CDateTime (y, m, d, h, min, s);
	return *this;
}

long long int
CDateTime::operator - (const CDateTime &date) const
{
	return stamp - date.stamp;
}

CDateTime
CDateTime::operator + (int s) const
{
	CDateTime tmp;
	tmp.stamp = stamp + s;
	return tmp;
}

bool
CDateTime::operator == (const CDateTime &date) const
{
	return stamp == date.stamp;
}

bool
CDateTime::operator != (const CDateTime &date) const
{
	return stamp != date.stamp;
}

bool
CDateTime::operator <= (const CDateTime &date) const
{
	return stamp <= date.stamp;
}

bool
CDateTime::operator >= (const CDateTime &date) const
{
	return stamp >= date.stamp;
}

bool
CDateTime::operator <  (const CDateTime &date) const
{
	return stamp < date.stamp;
}

bool
CDateTime::operator >  (const CDateTime &date) const
{
	return stamp > date.stamp;
}

int
CDateTime::operator [] (const char *part) const
{
	int y, m, d, h, min, s;
	decompose (y, m, d, h, min, s);

	if      (!strcmp (part, "year"))   return y;
	else if (!strcmp (part, "month"))  return m;
	else if (!strcmp (part, "day"))    return d;
	else if (!strcmp (part, "hour"))   return h;
	else if (!strcmp (part, "min"))    return min;
	else if (!strcmp (part, "sec"))    return s;
	else                               return 0;
}

std::istream &
operator >> (std::istream &is, CDateTime &date)
{
	int y, m, d, h, min, s;
	char p[] = {0, 0, 0, 0, 0, 0};

	ios::fmtflags f = is.flags ();
	is >> noskipws;
	is >> y >> p[0] >> m >> p[1] >> d >> p[2]
	   >> h >> p[3] >> min >> p[4] >> s;
	is.flags (f);

	if (!is.fail ())
	{
		if (strcmp (p, "-- ::")
			|| y < 1600 || y > 3000
			|| m < 1 || m > 12
			|| d < 1 || d > CDateTime::m_days[m] +
				(m == 2 && CDateTime::is_leap (y))
			|| h < 0 || h > 23
			|| min < 0 || min > 59
			|| s < 0 || s > 59)
			is.setstate (ios::failbit);
		else
			new (&date) CDateTime (y, m, d, h, min, s);
	}

	return is;
}

std::ostream &
operator << (std::ostream &os, const CDateTime &date)
{
	int y, m, d, h, min, s;
	date.decompose (y, m, d, h, min, s);

	char buf[40];
	snprintf (buf, sizeof buf,
		"%04d-%02d-%02d %02d:%02d:%02d",
		y, m, d, h, min, s);

	os << buf;
	return os;
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
	CDateTime     a, b;
	istringstream is;

	b = "2011-03-01 14:50:20";
	a = "2011-12-30 21:08:14";
	check_output (a, "2011-12-30 21:08:14");
	assert (a - b == 26288274);

	a = a + 259200;
	check_output (a, "2012-01-02 21:08:14");

	is . clear ();
	is . str ("2000-01-01 12:00:00");
	is >> a;
	assert (is.fail () == false);

	ostringstream os;
	os << a["year"]  << " ";
	os << a["month"] << " ";
	os << a["day"]   << " ";
	os << a["hour"]  << " ";
	os << a["min"]   << " ";
	os << a["sec"];
	assert (os.str () == "2000 1 1 12 0 0");

	assert (!(a == b));
	assert ( (a != b));
	assert (!(a >= b));
	assert ( (a <= b));
	assert (!(a >  b));
	assert ( (a <  b));

	a = "2000-12-31 00:00:00";
	is.clear ();
	is.str ("2010-13-27 00:00:00");
	is >> a;
	assert (is.fail () == true);

	is.clear ();
	is.str ("2003-02-29 00:00:00");
	is >> a;
	assert (is.fail () == true);

	is.clear ();
	is.str ("2004-2-29 1:2:3");
	is >> a;
	assert (is.fail () == false);

	is.clear ();
	is.str ("1900-02-29 00:00:00");
	is >> a;
	assert (is.fail () == true);

	is.clear ();
	is.str ("2000-02-29 00:00:00");
	is >> a;
	assert (is.fail () == false);

	is.clear ();
	is.str ("2000-01-01 12:60:00");
	is >> a;
	assert (is.fail () == true);


	a = "1700-01-01 00:00:00";
	check_output (a, "1700-01-01 00:00:00");
}
#endif /* ! __PROGTEST__ */

