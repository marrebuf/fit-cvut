#ifndef __PROGTEST__
#include <iostream>
#include <iomanip>
using namespace std;
#endif /* __PROGTEST__ */
 
class CList
{
public:
	            CList        (void);
	void        InsStart     (int val);
	void        InsEnd       (int val);
	void        RemoveFirst  (int val);

	void        Print        ();
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
CList::RemoveFirst (int val)
{
	TItem *iter;
	for (iter = m_First; iter; iter = iter->m_Next)
	{
		if (iter->m_Val != val)
			continue;

		if (iter->m_Next)
			iter->m_Next->m_Prev = iter->m_Prev;
		else
			m_Last = iter->m_Prev;

		if (iter->m_Prev)
			iter->m_Prev->m_Next = iter->m_Next;
		else
			m_First = iter->m_Next;

		delete iter;
		break;
	}
}

void
CList::Print ()
{
	bool first = true;
	TItem *iter;
	cout << "[";
	for (iter = m_First; iter; iter = iter->m_Next)
	{
		if (!first)  cout << ",";
		cout << iter->m_Val;
		first = false;
	}
	cout << "]" << endl;
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
	T1.RemoveFirst (3);
	T1.Print ();
	// [5,1,2,4]
	T1.RemoveFirst (1);
	T1.Print ();
	// [5,2,4]
	T1.RemoveFirst (4);
	T1.Print ();
	// [5,2]

	CList  T2;
	T2.InsStart (1);
	T2.Print ();
	// [1]
	T2.InsEnd (2);
	T2.Print ();
	// [1,2]
	T2.InsStart (3);
	T2.Print ();
	// [3,1,2]
	T2.InsEnd (1);
	T2.Print ();
	// [3,1,2,1]
	T2.RemoveFirst (1);
	T2.Print ();
	// [3,2,1]
	T2.RemoveFirst (2);
	T2.Print ();
	// [3,1]
	T2.RemoveFirst (3);
	T2.Print ();
	// [1]
	T2.RemoveFirst (3);
	T2.Print ();
	// [1]
	T2.RemoveFirst (1);
	T2.Print ();
	// []

	return 0;
}
#endif /* ! __PROGTEST__ */
