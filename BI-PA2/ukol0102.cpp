#ifndef __PROGTEST__
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#endif /* ! __PROGTEST__ */

using namespace std;

typedef unsigned long long ull;

// ===== Bit Writer ==========================================================
class BitWriter
{
	unsigned buffer_bits;
	unsigned char buffer;
	ostream &out;

public:
	BitWriter (ostream &s = cout);
	~BitWriter ();

	bool write (ull n, unsigned bits);
	void flush (unsigned value = 0);
};

BitWriter::BitWriter (ostream &s) : out (s)
{
	buffer_bits = buffer = 0;
}

BitWriter::~BitWriter ()
{
	flush ();
}

bool
BitWriter::write (ull n, unsigned bits)
{
	// Byte by byte, big endian
	if (bits > 8)
	{
		if (!write (n >> 8, bits - 8))
			return false;

		bits = 8;
		n &= 0xff;
	}

	// In case of overflow, push the byte out
	if (buffer_bits + bits >= 8)
	{
		int pushed_out = 8 - buffer_bits;
		buffer = (buffer <<         pushed_out )
		       | (n      >> (bits - pushed_out));

		out.write (reinterpret_cast<char *> (&buffer),
			sizeof buffer);
		if (!out.good ())
			return false;

		bits -= pushed_out;
		n &= (1 << bits) - 1;

		buffer_bits = buffer = 0;
	}

	buffer = (buffer << bits) | n;
	buffer_bits += bits;
	return true;
}

void
BitWriter::flush (unsigned value)
{
	if (!buffer_bits)
		return;

	int fill_bits = 8 - buffer_bits;
	if (value)  value = (1 << fill_bits) - 1;
	write (value, fill_bits);
}

// ===== Bit Reader ==========================================================
class BitReader
{
	istream &in;
	unsigned buffer_bits;
	unsigned char buffer, last_byte;
	unsigned n_read;

public:
	BitReader (istream &s = cin);

	bool read (ull &n, unsigned bits);
	unsigned readCount () const;
	unsigned lastByte () const;
};

BitReader::BitReader (istream &s) : in (s)
{
	buffer_bits = buffer = 0;
}

bool
BitReader::read (ull &n, unsigned bits)
{
	n_read = 0;

	while (bits--)
	{
		if (!buffer_bits)
		{
			in.read (reinterpret_cast<char *> (&buffer),
				sizeof buffer);
			if (!in.good ())
				return false;

			last_byte = buffer;
			buffer_bits = 8;
		}

		n = (n << 1) | (buffer >> 7);
		buffer <<= 1;
		buffer_bits--;
		n_read++;
	}

	return true;
}

unsigned
BitReader::readCount () const
{
	return n_read;
}

unsigned
BitReader::lastByte () const
{
	return last_byte;
}

// ===== Main Functions ======================================================
#define FUN_HEADER                        \
	ifstream in  (srcName, ios::binary);  \
	ofstream out (dstName, ios::binary);  \
	if (!in || !out)                      \
		return false;

static bool
eliasHelper (BitWriter &bw, ull n)
{
	if (n == 1)
		return true;

	int bits = sizeof n * 8 - 1;
	for (; bits >= 0; bits--)
		if (n & (1ULL << bits))
			break;

	if (!eliasHelper (bw, bits))
		return false;
	return bw.write (n, ++bits);
}

bool
toElias (const char *srcName, const char *dstName)
{
	FUN_HEADER
	BitWriter bw (out);

	while (true)
	{
		ull n;

		in.read (reinterpret_cast<char *> (&n),
			sizeof n);

		if (in.eof ())
		{
			bw.flush (1);
			return in.gcount () == 0;
		}

		if (!in.good ()
			|| !(n += 1)
			|| !eliasHelper (bw, n)
			|| !bw.write (0, 1))
			return false;
	}
}

bool
toBinary (const char *srcName, const char *dstName)
{
	FUN_HEADER
	BitReader br (in);

	while (true)
	{
		ull n = 1;
		unsigned read = 0;

		while (true)
		{
			ull x = 0;
			if (!br.read (x, 1))
			{
				if (n == 1)
					return true;

				unsigned bits = br.lastByte ()
					& ((1 << read) - 1);
				return !in.bad ()
					&& read < 8
					&& (bits & (bits + 1)) == 0;
			}

			if (!x)
				break;

			// Overflows unsigned long long
			if (n >= sizeof x * 8)
				return false;

			// If reading the next bits fails, see if what
			// we've read can be qualified as padding
			read++;
			if (!br.read (x, n))
			{
				read += br.readCount ();
				unsigned bits = br.lastByte ()
					& ((1 << read) - 1);
				return !in.bad ()
					&& read < 8
					&& (bits & (bits + 1)) == 0;
			}

			read += n;
			n = x;
		}

		out.write (reinterpret_cast<char *> (&--n),
			sizeof n);
		if (!out.good ())
			return false;
	}
}

// ===== Test Suite ==========================================================
#ifndef __PROGTEST__
#include <cassert>

static bool
check_file (const char *first, const char *second)
{
	ifstream f (first, ios::binary), s (second, ios::binary);
	if (!first || !second)
		return false;

	while (true)
	{
		int x = f.get ();
		int y = s.get ();

		if (f.eof () && s.eof ())
			return true;
		if (x != y)
			return false;
	}
}

static void
bit_print (const char *file)
{
	ifstream f (file, ios::binary);
	assert (f);

	while (true)
	{
		int c = f.get ();

		if (!f)
		{
			cout << endl;
			break;
		}

		for (int i = 7; i >= 0; i--)
			cout << char ('0' + ((c >> i) & 1));

		cout << ' ';
	}
}

int
main (int argc, char *argv [])
{
	cout << "Test for invalid files..." << endl;
	assert (!toElias  ("/nonexistent", "/nonexistent"));
	assert (!toBinary ("/nonexistent", "/nonexistent"));

	for (int i = 1; i <= 3; i++)
	{
		string ulong ("ulonglong");
		string elias ("elias");

		cout << "Test " << i << "..." << endl;

		ulong += i + '0';
		elias += i + '0';

		assert (toElias    (ulong.c_str (), (elias + "_gen").c_str ()));
		assert (check_file (elias.c_str (), (elias + "_gen").c_str ()));

		assert (toBinary   (elias.c_str (), (ulong + "_gen").c_str ()));
		assert (check_file (ulong.c_str (), (ulong + "_gen").c_str ()));
	}

	cout << "Test 4..." << endl;
	assert (!toBinary ("elias4", "elias4_tmp"));

	unsigned char a[6];

	for (int c = 1; c < 5; )
	{
		string name ("rand");
		name += c - 1 + 'A';

		string name_out (name + "_out");
		string name_back (name + "_back");

		ofstream f (name.c_str (), ios::binary);
		assert (f);

		for (unsigned i = 0; i < sizeof a; i++)
		{
			a[i] = rand () % 256;
			f << a[i];
		}

		f.close ();

		if (toBinary (name.c_str (), name_out.c_str()))
		{
			toElias (name_out.c_str (), name_back.c_str ());

			ifstream f (name_back.c_str (), ios::binary);
			assert (f);

			if (!check_file (name.c_str (), name_back.c_str ()))
			{
				cout << "Found error " << char ('A' + c - 1) << endl;
				cout << "Original   ";
				bit_print (name.c_str ());
				cout << "Generated  ";
				bit_print (name_back.c_str ());

				c++;
			}
		}
	}

	return 0;
}
#endif /* ! __PROGTEST__ */

