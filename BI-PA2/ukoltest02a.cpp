#ifndef __PROGTEST__
#include <iostream>
#include <iomanip>
using namespace std;
#endif /* __PROGTEST__ */
 
class CList
{
public:
	            CList          (void);
	void        InsStart       (int val);
	void        InsEnd         (int val);
	void        DuplicateEven  (void);

	void        Print          ();
protected:
	struct TItem
	{
		int     m_Val;
		TItem  *m_Prev;
		TItem  *m_Next;
	};
	TItem      *m_First;
	TItem      *m_Last;
};
 
CList::CList ()
{
	m_First = m_Last = 0;
}

void
CList::InsStart (int val)
{
	TItem *link = new TItem;
	link->m_Val = val;
	link->m_Prev = 0;
	link->m_Next = m_First;

	if (m_First)
		m_First->m_Prev = link;
	else
		m_Last = link;

	m_First = link;
}

void
CList::InsEnd (int val)
{
	TItem *link = new TItem;
	link->m_Val = val;
	link->m_Next = 0;
	link->m_Prev = m_Last;

	if (m_Last)
		m_Last->m_Next = link;
	else
		m_First = link;

	m_Last = link;
}

void
CList::DuplicateEven ()
{
	TItem *iter, *link;
	bool even = true;
	for (iter = m_First; iter; iter = iter->m_Next)
	{
		if ((even = !even))
			continue;

		link = new TItem (*iter);
		link->m_Prev = iter;
		link->m_Next = iter->m_Next;

		if (!iter->m_Next)
			m_Last = link;
		else
			iter->m_Next->m_Prev = link;
		iter = iter->m_Next = link;
	}
}

void
CList::Print ()
{
	bool first;
	TItem *iter;

	cout << "[";
	first = true;
	for (iter = m_First; iter; iter = iter->m_Next)
	{
		if (!first)  cout << ",";
		cout << iter->m_Val;
		first = false;
	}
	cout << "]" << endl;

	cout << "<";
	first = true;
	for (iter = m_Last; iter; iter = iter->m_Prev)
	{
		if (!first)  cout << ",";
		cout << iter->m_Val;
		first = false;
	}
	cout << ">" << endl;
}
 
#ifndef __PROGTEST__
int
main (int argc, char *argv[])
{
	CList  T1;
	T1.InsStart (1);
	T1.Print ();
	// [1]
	T1.InsEnd (2);
	T1.Print ();
	// [1,2]
	T1.InsStart (3);
	T1.Print ();
	// [3,1,2]
	T1.InsEnd (4);
	T1.Print ();
	// [3,1,2,4]
	T1.InsStart (5);
	T1.Print ();
	// [5,3,1,2,4]
	T1.DuplicateEven ();
	T1.Print ();
	// [5,5,3,1,1,2,4,4]
	T1.DuplicateEven ();
	T1.Print ();
	// [5,5,5,3,3,1,1,1,2,4,4,4]
	T1.DuplicateEven ();
	T1.Print ();
	// [5,5,5,5,5,3,3,3,1,1,1,1,2,2,4,4,4,4]

	CList  T2;
	T2.InsEnd (1);
	T2.Print ();
	// [1]
	T2.InsStart (2);
	T2.Print ();
	// [2,1]
	T2.InsStart (4);
	T2.Print ();
	// [4,2,1]
	T2.InsEnd (3);
	T2.Print ();
	// [4,2,1,3]
	T2.DuplicateEven ();
	T2.Print ();
	// [4,4,2,1,1,3]
	T2.DuplicateEven ();
	T2.Print ();
	// [4,4,4,2,2,1,1,1,3]
	T2.DuplicateEven ();
	T2.Print ();
	// [4,4,4,4,4,2,2,2,1,1,1,1,3,3]

	return 0;
}
#endif /* ! __PROGTEST__ */
