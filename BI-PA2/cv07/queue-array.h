// queue-array.h

#ifndef __QUEUE_ARRAY_H__
#define __QUEUE_ARRAY_H__

template <typename Elem>
class QueueArray
{
	Array<Elem> array;
	int head, len;

public:
	explicit QueueArray ();
	bool empty () const;
	QueueArray<Elem> &add (const Elem &el);
	Elem &front ();
	QueueArray<Elem> &remove ();
};

template <typename Elem>
QueueArray<Elem>::QueueArray ()
{
	head = len = 0;
}

template <typename Elem> bool
QueueArray<Elem>::empty () const
{
	return len == 0;
}

template <typename Elem> QueueArray<Elem> &
QueueArray<Elem>::add (const Elem &el)
{
	int array_len = array.length ();

	if (len == array_len)
	{
		int extension = array_len;

		array.extend (array_len + extension);
		for (int i = array_len; i-- > head; )
			array[i + extension] = array[i];

		head += extension;
		array_len += extension;
	}

	array[(head + len) % array_len] = el;
	len++;
	return *this;
}

template <typename Elem> Elem &
QueueArray<Elem>::front ()
{
	if (!len)
		throw "queue empty";
	return array[head];
}

template <typename Elem> QueueArray<Elem> &
QueueArray<Elem>::remove ()
{
	if (!len)
		return *this;

	head = (head + 1) % array.length ();
	len--;
	return *this;
}

#endif /* ! __QUEUE_ARRAY_H__ */

