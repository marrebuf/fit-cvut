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
	int get_id () {return _id;}
	void update (int x, int y, int w, int h);
	virtual void print (ostream &os, const char *indent) const = 0;
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
	virtual CControl *clone () const {return new CButton (*this);}
	virtual void print (ostream &os, const char *indent) const;
};

void
CButton::print (ostream &os, const char *indent) const
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
	virtual CControl *clone () const {return new CInput (*this);}
	CInput &SetValue (const char *x) {_value = x; return *this;}
	const char *GetValue () const {return _value.c_str ();}
	virtual void print (ostream &os, const char *indent) const;
};

void
CInput::print (ostream &os, const char *indent) const
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
	virtual CControl *clone () const {return new CLabel (*this);}
	virtual void print (ostream &os, const char *indent) const;
};

void
CLabel::print (ostream &os, const char *indent) const
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
	virtual CControl *clone () const {return new CComboBox (*this);}
	CComboBox &Add (const char *x);
	CComboBox &SetSelected (int x) {_selected = x; return *this;}
	int GetSelected () const {return _selected;}
	virtual void print (ostream &os, const char *indent) const;
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
CComboBox::print (ostream &os, const char *indent) const
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

class CWindow
{
	int _x, _y, _w, _h;
	string _title;

	CControl **_ctrls;
	int _n_ctrls, _size_ctrls;
public:
	CWindow (const char *title, int x, int y, int w, int h);
	CWindow (const CWindow &x);
	CWindow &operator = (const CWindow &x);
	~CWindow ();
	CWindow &Add (const CControl &x);
	CWindow &SetPosition (int x, int y, int w, int h);
	CControl *Search (int id) const;
	friend ostream &operator << (ostream &os, const CWindow &self);
};

CWindow::CWindow (const char *title, int x, int y, int w, int h)
	: _x (x), _y (y), _w (w), _h (h), _title (title)
{
	_ctrls = (CControl **) malloc ((_size_ctrls = 4) * sizeof *_ctrls);
	_n_ctrls = 0;
}

CWindow::CWindow (const CWindow &x)
	: _x (x._x), _y (x._y), _w (x._w), _h (x._h), _title (x._title)
{
	_n_ctrls = x._n_ctrls;
	_size_ctrls = x._size_ctrls;

	_ctrls = (CControl **) malloc (_size_ctrls * sizeof *_ctrls);
	for (int i = 0; i < _n_ctrls; i++)
		_ctrls[i] = x._ctrls[i]->clone ();
}

CWindow &
CWindow::operator = (const CWindow &x)
{
	if (this != &x)
	{
		this->~CWindow ();
		new (this) CWindow (x);
	}
	return *this;
}

CWindow::~CWindow ()
{
	for (int i = 0; i < _n_ctrls; i++)
		delete _ctrls[i];
	free (_ctrls);
}

CWindow &
CWindow::Add (const CControl &x)
{
	if (_n_ctrls == _size_ctrls)
		_ctrls = (CControl **) realloc (_ctrls,
			(_size_ctrls <<= 1) * sizeof *_ctrls);
	(_ctrls[_n_ctrls++] = x.clone ())->update (_x, _y, _w, _h);
	return *this;
}

CWindow &
CWindow::SetPosition (int x, int y, int w, int h)
{
	_x = x;
	_y = y;
	_w = w;
	_h = h;

	for (int i = 0; i < _n_ctrls; i++)
		_ctrls[i]->update (x, y, w, h);
	return *this;
}

CControl *
CWindow::Search (int id) const
{
	for (int i = 0; i < _n_ctrls; i++)
		if (_ctrls[i]->get_id () == id)
			return _ctrls[i];
	return NULL;
}

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
	a.Add (CComboBox (20, 0.1, 0.3, 0.8, 0.1)
		.Add ("Karate")
		.Add ("Judo")
		.Add ("Box")
		.Add ("Progtest"));
	cout << a;

	CWindow b = a;
	CControl *ctl = b.Search (20);
	cout << *ctl;

	CComboBox *cb = dynamic_cast<CComboBox *> (b.Search (20));
	cb->SetSelected (3);
	CInput *il = dynamic_cast<CInput *> (b.Search (11));
	il->SetValue ("chucknorris@fit.cvut.cz");
	b.Add (CComboBox (21, 0.1, 0.5, 0.8, 0.1)
		.Add ("PA2")
		.Add ("OSY")
		.Add ("Both"));
	cout << b;

	b.SetPosition (20, 30, 640, 520);
	cout << b;

	return 0;
}
#endif /* ! __PROGTEST__ */

