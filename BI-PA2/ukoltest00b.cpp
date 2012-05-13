#ifndef __PROGTEST__
#include <iostream>
#include <iomanip>
using namespace std;
#endif /* __PROGTEST__ */
 
template <class T>
class CList
{
public:
	            CList     (void);
	void        InsStart  (const T &val);
	void        InsEnd    (const T &val);
	void        InsAfter  (const T &existingVal, const T &newVal);

	void        Print     ();
protected:
	struct TItem
	{
		T       m_Val;
		TItem  *m_Prev;
		TItem  *m_Next;

		TItem (const T &val) : m_Val (val) {}
	};
	TItem      *m_First;
	TItem      *m_Last;
};
 
template <class T>
CList<T>::CList ()
{
	m_First = m_Last = 0;
}

template <class T>
void
CList<T>::InsStart (const T &val)
{
	TItem *link = new TItem (val);
	link->m_Prev = 0;
	link->m_Next = m_First;

	if (m_First)
		m_First->m_Prev = link;
	else
		m_Last = link;

	m_First = link;
}

template <class T>
void
CList<T>::InsEnd (const T &val)
{
	TItem *link = new TItem (val);
	link->m_Next = 0;
	link->m_Prev = m_Last;

	if (m_Last)
		m_Last->m_Next = link;
	else
		m_First = link;

	m_Last = link;
}

template <class T>
void
CList<T>::InsAfter (const T &existingVal, const T &newVal)
{
	TItem *iter, *link;
	for (iter = m_First; iter; iter = iter->m_Next)
	{
		if (iter->m_Val != existingVal)
			continue;

		link = new TItem (newVal);
		link->m_Prev = iter;
		link->m_Next = iter->m_Next;

		if (iter->m_Next)
			iter->m_Next->m_Prev = link;
		else
			m_Last = link;
		iter = iter->m_Next = link;
	}
}

template <class T>
void
CList<T>::Print ()
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
	CList<int>  T1;
	T1.InsStart (1);
	T1.Print ();
	// [1]
	T1.InsEnd (2);
	T1.Print ();
	// [1,2]
	T1.InsStart (2);
	T1.Print ();
	// [2,1,2]
	T1.InsEnd (1);
	T1.Print ();
	// [2,1,2,1]
	T1.InsStart (1);
	T1.Print ();
	// [1,2,1,2,1]
	T1.InsAfter (1, 3);
	T1.Print ();
	// [1,3,2,1,3,2,1,3]
	T1.InsAfter (1, 2);
	T1.Print ();
	// [1,2,3,2,1,2,3,2,1,2,3]
	T1.InsAfter (2, 5);
	T1.Print ();
	// [1,2,5,3,2,5,1,2,5,3,2,5,1,2,5,3]
	 
	CList<int>  T2;
	T2.InsStart (1);
	T2.Print ();
	// [1]
	T2.InsEnd (1);
	T2.Print ();
	// [1,1]
	T2.InsStart (1);
	T2.Print ();
	// [1,1,1]
	T2.InsEnd (1);
	T2.Print ();
	// [1,1,1,1]
	T2.InsAfter (1, 1);
	T2.Print ();
	// [1,1,1,1,1,1,1,1]
	T2.InsAfter (1, 1);
	T2.Print ();
	// [1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1]

	return 0;
}
#endif /* ! __PROGTEST__ */
