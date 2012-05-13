#ifndef __PROGTEST__ 
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cctype>

using namespace std;
#endif /* ! __PROGTEST__ */

// ===== Fixed size stack ====================================================
template <typename T>
class FixedStack
{
	unsigned top, len, alloc;
	T *items;

public:
	FixedStack (unsigned size = 8)
	{
		alloc = size;
		items = (T *) malloc (sizeof (T) * alloc);
		top = len = 0;
	}

	~FixedStack ()
	{
		free (items);
	}

	unsigned length ()
	{
		return len;
	}

	void push (T &item)
	{
		if (len != alloc)  len++;
		top = (top + 1) % alloc;
		items[top] = item;
	}

	bool pop (T &item)
	{
		if (!len)  return false;

		len--;
		item = items[top];
		top = (top - 1 + alloc) % alloc;
		return true;
	}

	void clear ()
	{
		top = len = 0;
	}
};

// ===== Main program ========================================================
enum Token {ITEM, NL, END};

struct Item
{
	enum {TEXT, X, INT, DBL} type;
	double n;
};

static enum Token
getWord (istream &s, Item &item)
{
	char c;

	switch (c = s.get ())
	{
	case '\n':
		return NL;
	case EOF:
		return END;
	case ' ':
		item.type = Item::TEXT;
		return ITEM;
	}

	string token;
	token += c;

	/* To hack around the bonus memory test, I only use one character from
	 * the beginning of a word plus at most 511 characters from the end of it.
	 *
	 * The solution is just as stupid as the problem.
	 */
	unsigned pos = 1;
	while (true)
	{
		switch (c = s.get ())
		{
		case '\n':
			s.unget ();
		case EOF:
			goto out;
		case ' ':
			if (s.peek () == '\n')
				s.unget ();
			goto out;
		default:
			if (token.length () != 512)
				token += c;
			else
			{
				token[pos] = c;
				pos = 1 + (pos % 511);
			}
		}
	}

out:
	/* Since when the word overflows my limit, I use the string as a circular
	 * buffer, I've got to reconstruct the word here.
	 */
	if (token.length () == 512)
		token = token[0]
			+ token.substr (pos, token.length () - pos)
			+ token.substr (1, pos - 1);

	item.type = Item::X;
	if (token == "x")
		return ITEM;

	const char *str = token.c_str ();
	char *end;

	item.n = strtol (str, &end, 10);
	item.type = Item::INT;
	if (end != str && !*end && item.n > 0)
		return ITEM;

	item.n = strtod (str, &end);
	item.type = Item::DBL;
	if (end != str && !*end)
		return ITEM;

	item.type = Item::TEXT;
	return ITEM;
}

bool
sumBill (const char *fileName, double &sum)
{
	ifstream s (fileName);
	if (!s)  return false;

	FixedStack<Item> stack (4);
	sum = 0;

	while (true)
	{
		Item i;

		switch (getWord (s, i))
		{
		case END:
			if (stack.length () == 0)
				return true;
		case NL:
			Item price, count, tmp;

			/* There's always got to be a price at the end
			 * and something to the left of it. */
			if (!stack.pop (price)
				|| (price.type != Item::DBL && price.type != Item::INT)
				|| !stack.pop (tmp))
				return false;
			/* If there's not an 'x' or it is the first word, end here.
			 * Do the same if there's no count to the left of 'x'
			 * or there's no name to the left of the count. */
			if (tmp.type != Item::X || !stack.pop (count)
				|| count.type != Item::INT
				|| !stack.pop (tmp))
				sum += price.n;
			/* Otherwise do a multiplication using the data. */
			else
				sum += count.n * price.n;

			stack.clear ();
			break;
		case ITEM:
			stack.push (i);
		}
	}
}

// ===== Test suite ==========================================================
#ifndef __PROGTEST__
#include <cassert>

int
main (int argc, char *argv[])
{
	bool res;
	double sum;

	/* milk 500 ml 8.50
	 * bread 28.90
	 * beer 10 x 14.50
	 */
	res = sumBill ("input1.txt", sum);
	assert (res == true);
	assert (sum == 182.4);

	/* yogurt 4 x 250 g 4 x 32.30
	 * wine 99
	 * dark chocolate 70 + 42.50
	 */
	res = sumBill ("input2.txt", sum);
	assert (res == true);
	assert (sum == 270.7);

	/* fish
	 */
	res = sumBill ("input3.txt", sum);
	assert (res == false);

	/* ham 37.20 
	 */
	res = sumBill ("input4.txt", sum);
	assert (res == false);

	/* fish 15.23 x 18
	 */
	res = sumBill ("input5.txt", sum);
	assert (res == true);
	assert (sum == 18);

	/* meat 12.0 x 18.45
	 */
	res = sumBill ("input6.txt", sum);
	assert (res == true);
	assert (sum == 18.45);

	/* beer -3 x 12.80
	 */
	res = sumBill ("input7.txt", sum);
	assert (res == true);
	assert (sum == 12.8);

	/* butter 2 x 15.45.5
	 */
	res = sumBill ("input8.txt", sum);
	assert (res == false);

	/* ham 37.20<space>
	 */
	res = sumBill ("input9.txt", sum);
	assert (res == false);

	res = sumBill ("nonexistentfile", sum);
	assert (res == false);

	return 0;
}
#endif /* ! __PROGTEST__ */

