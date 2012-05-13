#ifndef __PROGTEST__
#include <iostream>
#include <iomanip>
#include <string>
#include <cstdio>
#include <cstdlib>
using namespace std;
#endif /* __PROGTEST__ */

class CControl
{
protected:
	int _id;
	float _x, _y, _w, _h;
	int _xp, _yp, _wp, _hp;
public:
	CControl (int id, float x, float y, float w, float h,
		int xp = 0, int yp = 0, int wp = 0, int hp = 0)
		: _id (id), _x (x), _y (y), _w (w), _h (h),
		_xp (xp), _yp (yp), _wp (wp), _hp (hp) {}
	virtual ~CControl () {};
	virtual CControl *clone () const = 0;
	virtual CControl *search_id (int id)
		{return _id == id ? this : NULL;}
	virtual void update (int x, int y, int w, int h);
	virtual void print (ostream &os, const string &indent) const = 0;
	friend ostream &operator << (ostream &os, const CControl &self);
};

void
CControl::update (int x, int y, int w, int h)
{
	_xp = x + _x * w;
	_yp = y + _y * h;
	_wp = _w * w;
	_hp = _h * h;
}

ostream &
operator << (ostream &os, const CControl &self)
{
	self.print (os, "");
	return os;
}

class CButton : public CControl
{
	string _label;
public:
	CButton (int id, float x, float y, float w, float h, const char *label)
		: CControl (id, x, y, w, h), _label (label) {}
	virtual CControl *clone () const
		{return new CButton (*this);}
	virtual void print (ostream &os, const string &indent) const;
};

void
CButton::print (ostream &os, const string &indent) const
{
	os << "[" << _id << "]" << " Button \"" << _label << "\" ("
	   << _xp << "," << _yp << "," << _wp << "," << _hp << ")" << endl;
}

class CInput : public CControl
{
	string _value;
public:
	CInput (int id, float x, float y, float w, float h, const char *value)
		: CControl (id, x, y, w, h), _value (value) {}
	virtual CControl *clone () const
		{return new CInput (*this);}
	CInput &SetValue (const char *x)
		{_value = x; return *this;}
	const char *GetValue () const
		{return _value.c_str ();}
	virtual void print (ostream &os, const string &indent) const;
};

void
CInput::print (ostream &os, const string &indent) const
{
	os << "[" << _id << "]" << " Input \"" << _value << "\" ("
	   << _xp << "," << _yp << "," << _wp << "," << _hp << ")" << endl;
}

class CLabel : public CControl
{
	string _label;
public:
	CLabel (int id, float x, float y, float w, float h, const char *label)
		: CControl (id, x, y, w, h), _label (label) {}
	virtual CControl *clone () const
		{return new CLabel (*this);}
	virtual void print (ostream &os, const string &indent) const;
};

void
CLabel::print (ostream &os, const string &indent) const
{
	os << "[" << _id << "]" << " Label \"" << _label << "\" ("
	   << _xp << "," << _yp << "," << _wp << "," << _hp << ")" << endl;
}

class CComboBox : public CControl
{
	int _selected;

	string **_items;
	int _n_items, _size_items;
public:
	CComboBox (int id, float x, float y, float w, float h);
	CComboBox (const CComboBox &x);
	CComboBox &operator = (const CComboBox &x);
	virtual ~CComboBox ();
	virtual CControl *clone () const
		{return new CComboBox (*this);}
	CComboBox &Add (const char *x);
	CComboBox &SetSelected (int x)
		{_selected = x; return *this;}
	int GetSelected () const
		{return _selected;}
	virtual void print (ostream &os, const string &indent) const;
};

CComboBox::CComboBox (int id, float x, float y, float w, float h)
	: CControl (id, x, y, w, h), _selected (0)
{
	_items = (string **) malloc ((_size_items = 4) * sizeof *_items);
	_n_items = 0;
}

CComboBox::CComboBox (const CComboBox &x)
	: CControl (x._id, x._x, x._y, x._w, x._h, x._xp, x._yp, x._wp, x._hp)
{
	_selected = x._selected;
	_n_items = x._n_items;
	_size_items = x._size_items;

	_items = (string **) malloc (_size_items * sizeof *_items);
	for (int i = 0; i < _n_items; i++)
		_items[i] = new string (*x._items[i]);
}

CComboBox &
CComboBox::operator = (const CComboBox &x)
{
	if (this != &x)
	{
		this->~CComboBox ();
		new (this) CComboBox (x);
	}
	return *this;
}

CComboBox::~CComboBox ()
{
	for (int i = 0; i < _n_items; i++)
		delete _items[i];
	free (_items);
}

CComboBox &
CComboBox::Add (const char *x)
{
	if (_n_items == _size_items)
		_items = (string **) realloc (_items,
			(_size_items <<= 1) * sizeof *_items);
	_items[_n_items++] = new string (x);
	return *this;
}

void
CComboBox::print (ostream &os, const string &indent) const
{
	os << "[" << _id << "]" << " ComboBox ("
	   << _xp << "," << _yp << "," << _wp << "," << _hp << ")" << endl;

	for (int i = 0; i < _n_items; i++)
	{
		os << indent << "+-";
		if (i == _selected)
			os << '>' << *_items[i] << '<' << endl;
		else
			os << ' ' << *_items[i] << endl;
	}
}

class CContainer
{
protected:
	int _x, _y, _w, _h;

	CControl **_ctrls;
	int _n_ctrls, _size_ctrls;
public:
	CContainer (int x = 0, int y = 0, int w = 0, int h = 0);
	CContainer (const CContainer &x);
	CContainer &operator = (const CContainer &x);
	~CContainer ();
	CContainer &Add (const CControl &x);
	CControl *Search (int id) const;
	CContainer &SetPosition (int x, int y, int w, int h);
};

CContainer::CContainer (int x, int y, int w, int h)
	: _x (x), _y (y), _w (w), _h (h)
{
	_ctrls = (CControl **) malloc ((_size_ctrls = 4) * sizeof *_ctrls);
	_n_ctrls = 0;
}

CContainer::CContainer (const CContainer &x)
	: _x (x._x), _y (x._y), _w (x._w), _h (x._h)
{
	_n_ctrls = x._n_ctrls;
	_size_ctrls = x._size_ctrls;

	_ctrls = (CControl **) malloc (_size_ctrls * sizeof *_ctrls);
	for (int i = 0; i < _n_ctrls; i++)
		_ctrls[i] = x._ctrls[i]->clone ();
}

CContainer &
CContainer::operator = (const CContainer &x)
{
	if (this != &x)
	{
		this->~CContainer ();
		new (this) CContainer (x);
	}
	return *this;
}

CContainer::~CContainer ()
{
	for (int i = 0; i < _n_ctrls; i++)
		delete _ctrls[i];
	free (_ctrls);
}

CContainer &
CContainer::Add (const CControl &x)
{
	if (_n_ctrls == _size_ctrls)
		_ctrls = (CControl **) realloc (_ctrls,
			(_size_ctrls <<= 1) * sizeof *_ctrls);
	(_ctrls[_n_ctrls++] = x.clone ())->update (_x, _y, _w, _h);
	return *this;
}

CControl *
CContainer::Search (int id) const
{
	for (int i = 0; i < _n_ctrls; i++)
	{
		CControl *ctrl = _ctrls[i]->search_id (id);
		if (ctrl)
			return ctrl;
	}
	return NULL;
}

CContainer &
CContainer::SetPosition (int x, int y, int w, int h)
{
	_x = x;
	_y = y;
	_w = w;
	_h = h;

	for (int i = 0; i < _n_ctrls; i++)
		_ctrls[i]->update (x, y, w, h);
	return *this;
}

class CPanel : public CControl, public CContainer
{
public:
	CPanel (int id, float x, float y, float w, float h)
		: CControl (id, x, y, w, h) {}
	virtual CControl *clone () const
		{return new CPanel (*this);}
	virtual void update (int x, int y, int w, int h);
	virtual CControl *search_id (int id);
	virtual void print (ostream &os, const string &indent) const;
	CPanel &Add (const CControl &x)
		{CContainer::Add (x); return *this;}
};

void
CPanel::update (int x, int y, int w, int h)
{
	CControl::update (x, y, w, h);
	SetPosition (_xp, _yp, _wp, _hp);
}

CControl *
CPanel::search_id (int id)
{
	if (_id == id)
		return this;
	return Search (id);
}

void
CPanel::print (ostream &os, const string &indent) const
{
	os << "[" << _id << "]" << " Panel ("
	   << _xp << "," << _yp << "," << _wp << "," << _hp << ")" << endl;

	for (int i = 0; i < _n_ctrls; i++)
	{
		os << indent << "+- ";
		_ctrls[i]->print (os,
			indent + (i + 1 == _n_ctrls ? "   " : "|  "));
	}
}

class CWindow : public CContainer
{
	string _title;
public:
	CWindow (const char *title, int x, int y, int w, int h)
		: CContainer (x, y, w, h), _title (title) {}
	CWindow (const CWindow &x)
		: CContainer (x), _title (x._title) {}
	CWindow &Add (const CControl &x)
		{CContainer::Add (x); return *this;}
	CWindow &SetPosition (int x, int y, int w, int h)
		{CContainer::SetPosition (x, y, w, h); return *this;}
	friend ostream &operator << (ostream &os, const CWindow &self);
};

ostream &
operator << (ostream &os, const CWindow &self)
{
	os << "Window \"" << self._title << "\" ("
	   << self._x << "," << self._y << ","
	   << self._w << "," << self._h << ")" << endl;

	for (int i = 0; i < self._n_ctrls; i++)
	{
		os << "+- ";
		self._ctrls[i]->print (os,
			i + 1 == self._n_ctrls ? "   " : "|  ");
	}
	return os;
}

#ifndef __PROGTEST__
int
main (int argc, char *argv[])
{
	CWindow a ("Sample window", 10, 10, 600, 480);
	a.Add (CButton (1, 0.1, 0.8, 0.3, 0.1, "Ok"))
	 .Add (CButton (2, 0.6, 0.8, 0.3, 0.1, "Cancel"));
	a.Add (CLabel (10, 0.1, 0.1, 0.2, 0.1, "Username:"));
	a.Add (CInput (11, 0.4, 0.1, 0.5, 0.1, "chucknorris"));
	a.Add (CPanel (12, 0.1, 0.3, 0.8, 0.7)
		.Add (CComboBox (20, 0.1, 0.3, 0.8, 0.1)
			.Add ("Karate")
			.Add ("Judo")
			.Add ("Box")
			.Add ("Progtest")));
	cout << a;

	CWindow b = a;
	CControl * ctl = b.Search (20);
	cout << *ctl;

	CComboBox * cb = dynamic_cast<CComboBox *> (b.Search (20));
	cb->SetSelected (3);
	CInput * il = dynamic_cast<CInput *> (b.Search (11));
	il->SetValue ("chucknorris@fit.cvut.cz");
	CPanel * pa = dynamic_cast<CPanel *> (b.Search (12));
	pa->Add (CComboBox (21, 0.1, 0.5, 0.8, 0.1)
		.Add ("PA2")
		.Add ("OSY")
		.Add ("Both"));
	cout << b;

	b.SetPosition (20, 30, 640, 520);
	cout << b;

	pa->Add (*pa);
	pa->Add (*pa);
	pa->Add (*pa);
	cout << b;
	cout << *pa;

	return 0;
}
#endif /* ! __PROGTEST__ */

