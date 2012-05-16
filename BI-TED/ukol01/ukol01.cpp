#include <iostream>
#include <iomanip>
#include <string>
#include <cstdio>
#include <cstdlib>
using namespace std;

/**
 *  @mainpage
 *
 *  GUI System is the ultimate library of C++ classes for representing GUI
 *  structures.  Based on designs by Ladislav Vagner, this product is as
 *  professional, as it can possibly get.
 *
 *  Next follows an example code excerpt:
 *  @code
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
 *  @endcode
 *
 *  As you can clearly see, it's all centered around the CWindow component,
 *  which serves as the root element for all of the other objects, encapsulating
 *  them in a compact tree, which can then be printed out.
 *
 *  Should you ever need to create further groups of controls that are somehow
 *  related to each other, that's when CPanel comes into play.  It derives from
 *  the same base class as CWindow but it masks itself as a regular CControl.
 *  @em "How clever!" I can hear you shout.  Indeed it is.
 *
 */

/** Base class for GUI controls. */
class CControl
{
protected:
	int _id;   //!< A presumably unique ID of the control.

	float _x;  //!< The X coordinate of the control relative to its container.
	float _y;  //!< The Y coordinate of the control relative to its container.
	float _w;  //!< The width of the control relative to its container's size.
	float _h;  //!< The height of the control relative to its container's size.

	int _xp;   //!< The absolute X coordinate in pixels.
	int _yp;   //!< The absolute Y coordinate in pixels.
	int _wp;   //!< The real width in pixels.
	int _hp;   //!< The real height in pixels.
public:
	/** Object constructor.
	 *  @param[in] id  Control ID.
	 *  @param[in] x   Relative X coordinate.
	 *  @param[in] y   Relative Y coordinate.
	 *  @param[in] w   Relative width.
	 *  @param[in] h   Relative height.
	 *  @param[in] xp  Absolute X coordinate.
	 *  @param[in] yp  Absolute Y coordinate.
	 *  @param[in] wp  Real width.
	 *  @param[in] hp  Real height.
	 */
	CControl (int id, float x, float y, float w, float h,
		int xp = 0, int yp = 0, int wp = 0, int hp = 0)
		: _id (id), _x (x), _y (y), _w (w), _h (h),
		_xp (xp), _yp (yp), _wp (wp), _hp (hp) {}
	virtual ~CControl () {};
	/** Make an identical copy of the object.
	  * @return The identical clone.
	  */
	virtual CControl *clone () const = 0;
	/** Search for a control within the subtree rooted at this control.
	 *  @param[in] id  The ID we're searching for.
	 *  @return The control if found, NULL otherwise.
	 */
	virtual CControl *search_id (int id)
		{return _id == id ? this : NULL;}
	/** Recalculate absolute coordinates.
	 *  @param[in] x  Absolute X coordinate of the container.
	 *  @param[in] y  Absolute Y coordinate of the container.
	 *  @param[in] w  Absolute width of the container.
	 *  @param[in] h  Absolute height of the container.
	 */
	virtual void update (int x, int y, int w, int h);
	/** Print ourselves into the output stream.
	 *  @param[in] os  The output stream.
	 *  @param[in] indent  Indentation.
	 */
	virtual void print (ostream &os, const string &indent) const = 0;
	/** Print a control into an output stream.
	 *  @param[in] os  The output stream.
	 *  @param[in] self  The control.
	 *  @return The stream.
	 */
	friend ostream &operator << (ostream &os, const CControl &self);
};

void
CControl::update (int x, int y, int w, int h)
{
}

ostream &
operator << (ostream &os, const CControl &self)
{
	return os;
}

/** Push-button.  The user can click on this. */
class CButton : public CControl
{
	string _label;  //!< Text displayed on the button.
public:
	/** Object constructor.
	 *  @param[in] id     Control ID.
	 *  @param[in] x      Relative X coordinate.
	 *  @param[in] y      Relative Y coordinate.
	 *  @param[in] w      Relative width.
	 *  @param[in] h      Relative height.
	 *  @param[in] label  The text on the button.
	 */
	CButton (int id, float x, float y, float w, float h, const char *label)
		: CControl (id, x, y, w, h), _label (label) {}
	virtual CControl *clone () const
		{return new CButton (*this);}
	virtual void print (ostream &os, const string &indent) const;
};

void
CButton::print (ostream &os, const string &indent) const
{
}

/** Text input.  The user can enter text into it. */
class CInput : public CControl
{
	string _value;  //<! The value of the input.
public:
	/** Object constructor.
	 *  @param[in] id     Control ID.
	 *  @param[in] x      Relative X coordinate.
	 *  @param[in] y      Relative Y coordinate.
	 *  @param[in] w      Relative width.
	 *  @param[in] h      Relative height.
	 *  @param[in] value  The initial value.
	 */
	CInput (int id, float x, float y, float w, float h, const char *value)
		: CControl (id, x, y, w, h), _value (value) {}
	virtual CControl *clone () const
		{return new CInput (*this);}
	/** Set the value of the input.
	 *  @param[in] x  The new value.
	 *  @return Itself.
	 */
	CInput &SetValue (const char *x)
		{_value = x; return *this;}
	/** Return the value of the input.
	 *  @return The value.
	 */
	const char *GetValue () const
		{return _value.c_str ();}
	virtual void print (ostream &os, const string &indent) const;
};

void
CInput::print (ostream &os, const string &indent) const
{
}

/** Text label.  Used to display information to the user. */
class CLabel : public CControl
{
	string _value;  //<! The text of the label.
	string _label;
public:
	/** Object constructor.
	 *  @param[in] id     Control ID.
	 *  @param[in] x      Relative X coordinate.
	 *  @param[in] y      Relative Y coordinate.
	 *  @param[in] w      Relative width.
	 *  @param[in] h      Relative height.
	 *  @param[in] label  The text on the label.
	 */
	CLabel (int id, float x, float y, float w, float h, const char *label)
		: CControl (id, x, y, w, h), _label (label) {}
	virtual CControl *clone () const
		{return new CLabel (*this);}
	virtual void print (ostream &os, const string &indent) const;
};

void
CLabel::print (ostream &os, const string &indent) const
{
}

/** Combobox.  Allows the user to choose from a set of choices. */
class CComboBox : public CControl
{
	int _selected;  //<! Index of the currently selected choice.

	string **_items;  //!< An array of choices.
	int _n_items;     //!< The current count of choices.
	int_size_items;   //!< Allocated size of @a _items.
public:
	/** Object constructor.
	 *  @param[in] id     Control ID.
	 *  @param[in] x      Relative X coordinate.
	 *  @param[in] y      Relative Y coordinate.
	 *  @param[in] w      Relative width.
	 *  @param[in] h      Relative height.
	 */
	CComboBox (int id, float x, float y, float w, float h);
	/** Object copy constructor.
	 *  @param[in] x  The CComboBox that's being cloned.
	 */
	CComboBox (const CComboBox &x);
	/** Accept the choices of another CComboBox object.
	 *  @param[in] x  The object we're copying from.
	 *  @return Itself.
	 */
	CComboBox &operator = (const CComboBox &x);
	virtual ~CComboBox ();
	virtual CControl *clone () const
		{return new CComboBox (*this);}
	/** Add a choice to the control.
	 *  @param[in] x  The choice to be added.
	 *  @return Itself.
	 */
	CComboBox &Add (const char *x);
	/** Set the currently selected choice to @a x.
	 *  @return Itself.
	 */
	CComboBox &SetSelected (int x)
		{_selected = x; return *this;}
	/** Get the currently selected choice.
	 *  @return Index of the currently selected choice.
	 */
	int GetSelected () const
		{return _selected;}
	virtual void print (ostream &os, const string &indent) const;
};

CComboBox::CComboBox (int id, float x, float y, float w, float h)
	: CControl (id, x, y, w, h), _selected (0)
{
}

CComboBox::CComboBox (const CComboBox &x)
	: CControl (x._id, x._x, x._y, x._w, x._h, x._xp, x._yp, x._wp, x._hp)
{
}

CComboBox &
CComboBox::operator = (const CComboBox &x)
{
	return *this;
}

CComboBox::~CComboBox ()
{
}

CComboBox &
CComboBox::Add (const char *x)
{
	return *this;
}

void
CComboBox::print (ostream &os, const string &indent) const
{
}

/** Base class for GUI controls containers. */
class CContainer
{
protected:
	int _x;  //!< Absolute X coordinate.
	int _y;  //!< Absolute Y coordinate.
	int _w;  //!< Width in pixels.
	int _h;  //!< Height in pixels.

	CControl **_ctrls;  //!< An array of contained controls.
	int _n_ctrls;       //!< The current count of controls.
	int _size_ctrls;    //!< Allocated size of @a _ctrls.
public:
	/** Object constructor.
	 *  @param[in] x  Absolute X coordinate.
	 *  @param[in] y  Absolute Y coordinate.
	 *  @param[in] w  Width in pixels.
	 *  @param[in] h  Height in pixels.
	 */
	CContainer (int x = 0, int y = 0, int w = 0, int h = 0);
	/** Object copy constructor.
	 *  @param[in] x  The container that's being cloned.
	 */
	CContainer (const CContainer &x);
	/** Accept the contents of another container.
	 *  @param[in] x  The container we're reading the conrols from.
	 *  @return Itself.
	 */
	CContainer &operator = (const CContainer &x);
	~CContainer ();
	/** Add a control.
	 *  @param[in] x  The control to be added.
	 *  @return Itself.
	 */
	CContainer &Add (const CControl &x);
	/** Search for a control.
	 *  @param[in] id  The ID to be searched for.
	 *  @return The control if found, NULL otherwise.
	 */
	CControl *Search (int id) const;
	/** Reset the position.
	 *  @param[in] x  The new X coordinate.
	 *  @param[in] y  The new Y coordinate.
	 *  @param[in] w  The new width.
	 *  @param[in] h  The new height.
	 *  @return Itself.
	 */
	CContainer &SetPosition (int x, int y, int w, int h);
};

CContainer::CContainer (int x, int y, int w, int h)
	: _x (x), _y (y), _w (w), _h (h)
{
}

CContainer::CContainer (const CContainer &x)
	: _x (x._x), _y (x._y), _w (x._w), _h (x._h)
{
}

CContainer &
CContainer::operator = (const CContainer &x)
{
	return *this;
}

CContainer::~CContainer ()
{
}

CContainer &
CContainer::Add (const CControl &x)
{
	return *this;
}

CControl *
CContainer::Search (int id) const
{
	return NULL;
}

CContainer &
CContainer::SetPosition (int x, int y, int w, int h)
{
	return *this;
}

/** Independent nestable container for grouping controls. */
class CPanel : public CControl, public CContainer
{
public:
	/** Object constructor.
	 *  @param[in] id     Control ID.
	 *  @param[in] x      Relative X coordinate.
	 *  @param[in] y      Relative Y coordinate.
	 *  @param[in] w      Relative width.
	 *  @param[in] h      Relative height.
	 */
	CPanel (int id, float x, float y, float w, float h)
		: CControl (id, x, y, w, h) {}
	virtual CControl *clone () const
		{return new CPanel (*this);}
	virtual void update (int x, int y, int w, int h);
	virtual CControl *search_id (int id);
	virtual void print (ostream &os, const string &indent) const;
	/** Wrapper for CContainer::Add(). */
	CPanel &Add (const CControl &x)
		{CContainer::Add (x); return *this;}
};

void
CPanel::update (int x, int y, int w, int h)
{
}

CControl *
CPanel::search_id (int id)
{
	return Search (id);
}

void
CPanel::print (ostream &os, const string &indent) const
{
}

/** Top-level container for controls. */
class CWindow : public CContainer
{
	string _title;
public:
	/** Object constructor.
	 *  @param[in] title  Title of the window.
	 *  @param[in] x      Absolute X coordinate.
	 *  @param[in] y      Absolute Y coordinate.
	 *  @param[in] w      Width in pixels.
	 *  @param[in] h      Height in pixels.
	 */
	CWindow (const char *title, int x, int y, int w, int h)
		: CContainer (x, y, w, h), _title (title) {}
	/** Object copy constructor.
	 *  @param[in] x  The window that's being cloned.
	 */
	CWindow (const CWindow &x)
		: CContainer (x), _title (x._title) {}
	/** Wrapper for CContainer::Add(). */
	CWindow &Add (const CControl &x)
		{CContainer::Add (x); return *this;}
	/** Wrapper for CContainer::SetPosition(). */
	CWindow &SetPosition (int x, int y, int w, int h)
		{CContainer::SetPosition (x, y, w, h); return *this;}
	/** Print a window into an output stream.
	 *  @param[in] os  The output stream.
	 *  @param[in] self  The window.
	 *  @return The stream.
	 */
	friend ostream &operator << (ostream &os, const CWindow &self);
};

ostream &
operator << (ostream &os, const CWindow &self)
{
	return os;
}

int
main (int argc, char *argv[])
{
	return 0;
}

