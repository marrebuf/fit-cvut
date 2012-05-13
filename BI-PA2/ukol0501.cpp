#ifndef __PROGTEST__
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
using namespace std;

class InvalidIndexException
{
public:
	InvalidIndexException (int idx) {m_Idx = idx;}
    friend ostream &operator << (ostream &os, const InvalidIndexException &x)
    {
		return os << x.m_Idx;
	}
private:
	int m_Idx;
};

#endif /* __PROGTEST__ */

template <class T>
class CSparseArray
{
protected:
	int size;
	struct TElem
	{
		TElem *m_Next;
		int m_Idx; 
		T m_Val; 

		TElem (const T &x, TElem *next = NULL, int idx = 0)
			: m_Val (x) {m_Idx = idx; m_Next = next;}
	}
	*m_First, *m_Last; 

public:
	~CSparseArray ()
	{
		while (m_First)
		{
			TElem *el = m_First;
			m_First = el->m_Next;
			delete el;
		}

		m_First = m_Last = NULL;
		size = 0;
	}

	CSparseArray ()
	{
		m_First = m_Last = NULL;
		size = 0;
	}

	CSparseArray (const CSparseArray<T> &a)
	{
		m_First = m_Last = NULL;
		size = a.size;

		for (TElem *iter = a.m_First; iter; iter = iter->m_Next)
		{
			TElem *el = new TElem (iter->m_Val, NULL, iter->m_Idx);

			if (m_First)
				m_Last = m_Last->m_Next = el;
			else
				m_Last = m_First = el;
		}
	}

	CSparseArray<T> &operator = (const CSparseArray<T> &a)
	{
		if (this != &a)
		{
			this->~CSparseArray ();
			new (this) CSparseArray<T> (a);
		}
		return *this;
	}

	CSparseArray<T> &Set (int index, const T &x)
	{
		TElem **el = &m_First;
		for (; *el; el = &(*el)->m_Next)
		{
			int diff = (*el)->m_Idx - index;
			if (diff >  0)
				break;
			if (diff == 0)
			{
				(*el)->m_Val = x;
				return *this;
			}
		}

		TElem *next = *el;
		*el = new TElem (x, next, index);
		if (!next)
			m_Last = *el;
		size++;
		return *this;
	}

	CSparseArray<T> &Unset (int index)
	{
		TElem **el = &m_First, *prev = NULL;
		for (; *el; el = &(*el)->m_Next)
		{
			if ((*el)->m_Idx == index)
				break;
			prev = *el;
		}
		if (!*el)
			return *this;

		TElem *unlinked = *el;
		*el = unlinked->m_Next;
		delete unlinked;
		if (!*el)
			m_Last = prev;
		size--;
		return *this;
	}

	bool IsSet (int index) const
	{
		TElem *el = m_First;
		for (; el; el = el->m_Next)
			if (el->m_Idx == index)
				return true;
		return false;
	}

	int Size () const
	{
		return size;
	}

	bool operator == (const CSparseArray<T> &a) const
	{
		TElem *iter_a, *iter_b;
		iter_a = m_First;
		iter_b = a.m_First;
		while (iter_a || iter_b)
		{
			if (!iter_a || !iter_b)
				return false;
			if (iter_a->m_Idx != iter_b->m_Idx
			 || iter_a->m_Val != iter_b->m_Val)
				return false;
			iter_a = iter_a->m_Next;
			iter_b = iter_b->m_Next;
		}
		return true;
	}

	bool operator != (const CSparseArray<T> &a) const
	{
		return !(*this == a);
	}

	const T &operator [] (int index) const
	{
		const TElem *el = m_First;
		for (; el; el = el->m_Next)
			if (el->m_Idx == index)
				return el->m_Val;
		throw InvalidIndexException (index);
	}

	T &operator [] (int index)
	{
		TElem *el = m_First;
		for (; el; el = el->m_Next)
			if (el->m_Idx == index)
				return el->m_Val;
		throw InvalidIndexException (index);
	}

	friend std::ostream &operator << (std::ostream &os, const CSparseArray<T> &a)
	{
		bool first = true;
		typename CSparseArray<T>::TElem *el = a.m_First;

		os << "{ ";

		while (el)
		{
			if (!first)
				os << ", ";
			os << "[" << el->m_Idx;
			os << "] => " << el->m_Val;
			el = el->m_Next;
			first = false;
		}
		if (!first)
			os << " ";

		os << "}";
		return os;
	}
};

#ifndef __PROGTEST__
#include <cassert>
#include <sstream>

class CInt
{
	int *val;
public:
	CInt (int new_val) : val (new int (new_val)) {}
	CInt (const CInt &c) : val (new int (*c.val)) {}
	~CInt () {delete val;}
	bool operator == (const CInt &i) {return *i.val == *val;}
	bool operator != (const CInt &i) {return *i.val != *val;}

	CInt &operator = (const CInt &c)
	{
		if (this != &c)
			*val = *c.val;
		return *this;
	}

	friend std::ostream &operator << (std::ostream &os, const CInt &i)
	{
		return os << *i.val;
	}
};

template <class T>
class CTestArray : public CSparseArray<T>
{
	typedef typename CSparseArray<T>::TElem TElem;

public:
	void self_check () const
	{
		TElem *iter = this->m_First;
		if (!iter)
			assert (!this->m_Last);
		while (iter)
		{
			if (!iter->m_Next)
				assert (iter == this->m_Last);
			iter = iter->m_Next;
		}
	}
};

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
	CSparseArray<int>  A1;
	A1.Set (100, 20);
	A1.Set (30000, 80);
	A1.Set (-6000, 120);
	assert (A1.IsSet (0) == false);
	assert (A1.IsSet (30000) == true);
	assert (A1 [ 30000 ] == 80);
	assert (A1.Size () == 3);
	check_output (A1, "{ [-6000] => 120, [100] => 20, [30000] => 80 }");
	A1.Unset (100);
	assert (A1.Size () == 2);
	check_output (A1, "{ [-6000] => 120, [30000] => 80 }");
	A1.Set (30000, 666);
	assert (A1.Size () == 2);
	check_output (A1, "{ [-6000] => 120, [30000] => 666 }");
	try {
		A1 [ 30001 ];
		abort ();
	} catch (const InvalidIndexException &ex) {
		check_output (ex, "30001");
	} catch (...) {
		abort ();
	}

	CSparseArray<int>  A2;
	CSparseArray<int>  B2;
	A2.Set (5000, 20);
	A2.Set (30000, 80);
	A2.Set (-6000, 120);
	A2.Set (-9999999, 444);
	B2.Set (30000, 80);
	B2.Set (-6000, 120);
	B2.Set (-9999999, 444);

	check_output (A2,
		"{ [-9999999] => 444, [-6000] => 120, [5000] => 20, [30000] => 80 }");
	check_output (B2,
		"{ [-9999999] => 444, [-6000] => 120, [30000] => 80 }");
	assert (!(A2 == B2));
	assert (A2 != B2);

	B2.Set (5000, 20);
	B2.Set (5000, 20);

	check_output (A2,
		"{ [-9999999] => 444, [-6000] => 120, [5000] => 20, [30000] => 80 }");
	check_output (B2,
		"{ [-9999999] => 444, [-6000] => 120, [5000] => 20, [30000] => 80 }");
	assert (A2 == B2);
	assert (!(A2 != B2));

	CSparseArray<int>  A3;
	A3.Set (100, 0);
	A3.Set (99, 1);
	A3.Set (102, 2).Set (103, 2);
	check_output (A3, "{ [99] => 1, [100] => 0, [102] => 2, [103] => 2 }");
	A3.Unset (103);
	check_output (A3, "{ [99] => 1, [100] => 0, [102] => 2 }");
	A3.Unset (99).Unset (100);
	check_output (A3, "{ [102] => 2 }");
	A3.Unset (102);
	check_output (A3, "{ }");

	CSparseArray<string>  S;
	S.Set (25, "first");
	S.Set (36, "third");
	S.Set (30, "second");
	check_output (S, "{ [25] => first, [30] => second, [36] => third }");

	CSparseArray<string>  k (S);
	k = S;
	check_output (k, "{ [25] => first, [30] => second, [36] => third }");

	/* Brute force tests, very CPU-intensive. */
	#define ALEN 200
	#define AMOV -100
	#define INVL 99999

	CInt ref[200] = INVL;
	CTestArray<CInt>  *l = new CTestArray<CInt>;
	for (int i = -10000; i <= 10000; i++)
	{
		int idx = rand () % ALEN + AMOV;
		ref[idx - AMOV] = i;
		l->Set (idx, CInt (i));
		l->self_check ();
		assert (l->IsSet (idx));

		CInt tmp = (*l)[idx];
		assert (tmp == i);

		int size = l->Size ();
		idx = rand () % ALEN + AMOV;
		ref[idx - AMOV] = INVL;

		try
		{
			(*l)[idx];
			size--;
		}
		catch (const InvalidIndexException &ex)
		{
		}

		l->Unset (idx);
		l->self_check ();
		assert (!l->IsSet (idx));
		assert (l->Size () == size);

		CTestArray<CInt> *copy = new CTestArray<CInt>;

		*copy = *l;
		copy->self_check ();
		*copy = *copy;
		copy->self_check ();
		assert (*l == *copy);
		assert (!(*l != *copy));
		delete l;
		l = copy;
	}

	for (int i = 0; i < ALEN; i++)
	{
		if (ref[i] == INVL)
			assert (!l->IsSet (i + AMOV));
		else
			assert (ref[i] == (*l)[i + AMOV]);
	}

	cout << *l << endl;
	delete l;
}
#endif /* __PROGTEST__ */

