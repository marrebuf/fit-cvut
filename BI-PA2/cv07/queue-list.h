// queue-list.h

#ifndef __QUEUE_LIST_H__
#define __QUEUE_LIST_H__

template <typename Elem>
class QueueList
{
	struct Link
	{
		Elem el;
		Link *next;

		explicit
		Link (const Elem &el, Link *next = NULL)
			: el (el), next (next) {}
	};

private:
	Link *head, *tail;

public:
	explicit QueueList ();
	~QueueList ();
	bool empty () const;
	QueueList<Elem> &add (const Elem &el);
	Elem &front ();
	QueueList<Elem> &remove ();
};

template <typename Elem>
QueueList<Elem>::QueueList ()
{
	head = tail = NULL;
}

template <typename Elem>
QueueList<Elem>::~QueueList ()
{
	while (head)
	{
		Link *tmp = head;
		head = head->next;
		delete tmp;
	}
}

template <typename Elem> bool
QueueList<Elem>::empty () const
{
	return head == NULL;
}

template <typename Elem> QueueList<Elem> &
QueueList<Elem>::add (const Elem &el)
{
	if (!head)
		head = tail = new Link (el);
	else
		tail = tail->next = new Link (el);

	return *this;
}

template <typename Elem> Elem &
QueueList<Elem>::front ()
{
	if (!head)
		throw "queue empty";
	return head->el;
}

template <typename Elem> QueueList<Elem> &
QueueList<Elem>::remove ()
{
	if (!head)
		return *this;

	Link *tmp = head;
	head = head->next;
	delete tmp;

	if (!head)
		tail = NULL;

	return *this;
}

#endif /* ! __QUEUE_LIST_H__ */

