#ifndef __PROGTEST__
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <string>
#include <fstream>

#include <sstream>
#include <cassert>
using namespace std;


class InvalidCarException 
{
	string m_RZ;

public:
	InvalidCarException (const string &rz) : m_RZ (rz) {}

	friend ostream &operator << (ostream &os, const InvalidCarException &e) 
	{
		return os << e.m_RZ;
	}
};
	
class InvalidDriverException 
{
	string m_Name;
	string m_Surname;

public:
	InvalidDriverException (const string &name, const string &surname)
		: m_Name (name), m_Surname (surname) {}

	friend ostream &operator << (ostream &os, const InvalidDriverException &e) 
	{
		return os << e.m_Name << ' ' << e.m_Surname;
	}
};

class CTester;
#else /* __PROGTEST__ */
#define assert(cond)
#endif /* __PROGTEST__ */


class CDriver;
class CCar;
class CDatabase;

class CDriver
{
protected:
	struct TLog
	{
		string m_Date;
		CCar *m_Car;

		TLog () : m_Date (""), m_Car (NULL) {}
		TLog (const string &date, CCar *car)
			: m_Date (date), m_Car (car) {}

		TLog &operator = (const TLog &log)
		{
			m_Date = log.m_Date;
			m_Car = log.m_Car;
			return *this;
		}
	};

	string m_Name;
	string m_Surname;
	TLog *m_Log;
	int m_LogNr;

	int log_alloc;

public:
	CDriver (const string &name, const string &surname);
	~CDriver (void);
	friend std::ostream &operator << (std::ostream &os, const CDriver &driver);
	friend class ::CTester;
	friend class CCar;
	static int compare (CDriver **d1, CDriver **d2);
	static int log_compare (TLog *l1, TLog *l2);
	bool log_ride (const string &date, CCar *car, bool dry_run);
	void print (std::ostream &os, int indent = 0) const;
};

class CCar 
{
protected:
	struct TLog
	{
		string m_Date;
		CDriver *m_Driver;

		TLog () : m_Date (""), m_Driver (NULL) {}
		TLog (const string &date, CDriver *driver)
			: m_Date (date), m_Driver (driver) {}

		TLog &operator = (const TLog &log)
		{
			m_Date = log.m_Date;
			m_Driver = log.m_Driver;
			return *this;
		}
	};
	string m_RZ;
	TLog *m_Log;
	int m_LogNr;

	int log_alloc;

public:
	CCar (const string &RZ);
	~CCar (void);
	friend std::ostream &operator << (std::ostream &os, const CCar &car);
	friend class ::CTester;
	friend class CDriver;
	static int compare (CCar **c1, CCar **c2);
	static int log_compare (TLog *l1, TLog *l2);
	bool log_ride (const string &date, CDriver *driver, bool dry_run);
	void print (std::ostream &os, int indent = 0) const;
	void clone_into (CDatabase &db) const;
}; 

class CDatabase
{
public:
	CDatabase (void); 
	~CDatabase (void);
	bool Add (const string &date, const string &name,
		const string &surname, const string &RZ);
	const CCar &FindCar (const string &RZ) const;
	const CDriver &FindDriver (const string &name, const string &surname) const;
	friend std::ostream &operator << (std::ostream &os, const CDatabase &db);
	friend class ::CTester;
	CDatabase &operator = (const CDatabase &db);
	CDatabase (const CDatabase &db);

protected:
	CDriver **m_Drivers;
	int m_DriversNr;
	
	CCar **m_Cars;
	int m_CarsNr;

	int drivers_alloc;
	int cars_alloc;
}; 

// --- My superior generic binary search ---------------------------------------
template<typename T> bool
my_bsearch (T *key, T *base, size_t nmemb,
	T *&pos, int (*cmp) (T *key, T *item))
{
	int min = 0, max = nmemb - 1;

	while (max >= min)
	{
		int mid = (min + max) >> 1;
		int n = cmp (key, base + mid);

		if      (n < 0)
			min = mid + 1;
		else if (n > 0)
			max = mid - 1;
		else
		{
			pos = base + mid;
			return true;
		}
	}

	pos = base + min;
	return false;
}

#define PRINT_INDENT(s, n) for (int in = 0; in < (n); in++) (s) << ' ';
#define INDENT_CONST 3

// --- Driver, Car -------------------------------------------------------------
CDriver::CDriver (const string &name, const string &surname)
{
	m_Name = name;
	m_Surname = surname;

	m_LogNr = 0;
	m_Log = new TLog[log_alloc = 16];
}

CDriver::~CDriver (void)
{
	delete [] m_Log;
}

void
CDriver::print (std::ostream &os, int indent) const
{
	PRINT_INDENT (os, indent);
	os << m_Name << " " << m_Surname << endl;
	for (int i = 0; i < m_LogNr; i++)
	{
		PRINT_INDENT (os, indent + INDENT_CONST);
		os << m_Log[i].m_Date << ": " << m_Log[i].m_Car->m_RZ << endl;
	}
}

std::ostream &
operator << (std::ostream &os, const CDriver &driver)
{
	driver.print (os, 0);
	return os;
}

int
CDriver::compare (CDriver **d1, CDriver **d2)
{
	assert (d1 && d2);

	int name_cmp = (*d2)->m_Surname.compare ((*d1)->m_Surname);
	if (name_cmp)  return name_cmp;
	return (*d2)->m_Name.compare ((*d1)->m_Name);
}

int
CDriver::log_compare (TLog *l1, TLog *l2)
{
	assert (l1 && l2);
	return l2->m_Date.compare (l1->m_Date);
}

bool
CDriver::log_ride (const string &date, CCar *car, bool dry_run)
{
	TLog key (date, car);
	TLog *pos;

	if (my_bsearch (&key, m_Log, m_LogNr, pos, log_compare))
		return false;

	if (dry_run)
		return true;

	/* Enlarge the array, reduce to the general case. */
	if (m_LogNr >= log_alloc)
	{
		TLog *new_log = new TLog[log_alloc <<= 1];
		for (int i = 0; i < m_LogNr; i++)
			new_log[i] = m_Log[i];
		delete [] m_Log;
		pos = new_log + (pos - m_Log);
		m_Log = new_log;
	}

	for (TLog *iter = m_Log + m_LogNr; iter-- > pos; )
		iter[1] = iter[0];

	*pos = key;
	m_LogNr++;
	return true;
}

CCar::CCar (const string &RZ)
{
	m_RZ = RZ;

	m_LogNr = 0;
	m_Log = new TLog[log_alloc = 16];
}

CCar::~CCar (void)
{
	delete [] m_Log;
}

void
CCar::print (std::ostream &os, int indent) const
{
	PRINT_INDENT (os, indent);
	os << m_RZ << endl;
	for (int i = 0; i < m_LogNr; i++)
	{
		PRINT_INDENT (os, indent + INDENT_CONST);
		os << m_Log[i].m_Date << ": "
		   << m_Log[i].m_Driver->m_Name << " "
		   << m_Log[i].m_Driver->m_Surname << endl;
	}
}

std::ostream &
operator << (std::ostream &os, const CCar &car)
{
	car.print (os, 0);
	return os;
}

int
CCar::compare (CCar **c1, CCar **c2)
{
	assert (c1 && c2);
	return (*c2)->m_RZ.compare ((*c1)->m_RZ);
}

int
CCar::log_compare (TLog *l1, TLog *l2)
{
	assert (l1 && l2);
	return l2->m_Date.compare (l1->m_Date);
}

bool
CCar::log_ride (const string &date, CDriver *driver, bool dry_run)
{
	TLog key (date, driver);
	TLog *pos;

	if (my_bsearch (&key, m_Log, m_LogNr, pos, log_compare))
		return false;

	if (dry_run)
		return true;

	/* Enlarge the array, reduce to the general case. */
	if (m_LogNr >= log_alloc)
	{
		TLog *new_log = new TLog[log_alloc <<= 1];
		for (int i = 0; i < m_LogNr; i++)
			new_log[i] = m_Log[i];
		delete [] m_Log;
		pos = new_log + (pos - m_Log);
		m_Log = new_log;
	}

	for (TLog *iter = m_Log + m_LogNr; iter-- > pos; )
		iter[1] = iter[0];

	*pos = key;
	m_LogNr++;
	return true;
}

void
CCar::clone_into (CDatabase &db) const
{
	for (int i = 0; i < m_LogNr; i++)
	{
		const TLog &log = m_Log[i];
		db.Add (log.m_Date,
			log.m_Driver->m_Name, log.m_Driver->m_Surname, m_RZ);
	}
}

// --- Database ----------------------------------------------------------------
CDatabase::CDatabase (void)
{
	m_DriversNr = 0;
	m_Drivers = new CDriver *[drivers_alloc = 16];

	m_CarsNr = 0;
	m_Cars = new CCar *[cars_alloc = 16];
}

CDatabase::CDatabase (const CDatabase &db)
{
	m_DriversNr = 0;
	m_Drivers = new CDriver *[drivers_alloc = db.drivers_alloc];

	m_CarsNr = 0;
	m_Cars = new CCar *[cars_alloc = db.cars_alloc];

	for (int i = 0; i < db.m_CarsNr; i++)
		db.m_Cars[i]->clone_into (*this);
}

CDatabase::~CDatabase (void)
{
	for (int i = 0; i < m_DriversNr; i++)
		delete m_Drivers[i];
	delete [] m_Drivers;

	for (int i = 0; i < m_CarsNr; i++)
		delete m_Cars[i];
	delete [] m_Cars;
}

bool
CDatabase::Add (const string &date, const string &name,
	const string &surname, const string &RZ)
{
	CCar car_key (RZ),
		 *car_key_ptr = &car_key, **car;
	CDriver driver_key (name, surname),
		*driver_key_ptr = &driver_key, **driver;
	bool car_found, driver_found;

	car_found = my_bsearch (&car_key_ptr, m_Cars, m_CarsNr,
		car, CCar::compare);
	driver_found = my_bsearch (&driver_key_ptr, m_Drivers, m_DriversNr,
		driver, CDriver::compare);

	if (car_found    && !(*car)   ->log_ride (date, NULL, true))
		return false;
	if (driver_found && !(*driver)->log_ride (date, NULL, true))
		return false;

	if (!car_found)
	{
		/* Enlarge the array, reduce to the general case. */
		if (m_CarsNr >= cars_alloc)
		{
			CCar **new_cars = new CCar *[cars_alloc <<= 1];
			memcpy (new_cars, m_Cars, sizeof *m_Cars * m_CarsNr);
			delete [] m_Cars;
			car = new_cars + (car - m_Cars);
			m_Cars = new_cars;
		}

		memmove (car + 1,
			car, sizeof *m_Cars * (m_CarsNr - (car - m_Cars)));
		m_CarsNr++;
		*car = new CCar (RZ);
	}

	if (!driver_found)
	{
		/* Enlarge the array, reduce to the general case. */
		if (m_DriversNr >= drivers_alloc)
		{
			CDriver **new_drivers = new CDriver *[drivers_alloc <<= 1];
			memcpy (new_drivers, m_Drivers, sizeof *m_Drivers * m_DriversNr);
			delete [] m_Drivers;
			driver = new_drivers + (driver - m_Drivers);
			m_Drivers = new_drivers;
		}

		memmove (driver + 1, driver,
			sizeof *m_Drivers * (m_DriversNr - (driver - m_Drivers)));
		m_DriversNr++;
		*driver = new CDriver (name, surname);
	}

	assert (car    != NULL && *car    != NULL);
	assert (driver != NULL && *driver != NULL);

	return (*car)   ->log_ride (date, *driver, false)
	    && (*driver)->log_ride (date, *car, false);
}

const CCar &
CDatabase::FindCar (const string &RZ) const
{
	CCar key (RZ), *key_ptr = &key;
	CCar **car;

	if (!my_bsearch (&key_ptr, m_Cars, m_CarsNr,
		car, CCar::compare))
		throw InvalidCarException (RZ);
	else
		return **car;
}

const CDriver &
CDatabase::FindDriver (const string &name, const string &surname) const
{
	CDriver key (name, surname), *key_ptr = &key;
	CDriver **driver;

	if (!my_bsearch (&key_ptr, m_Drivers, m_DriversNr,
		driver, CDriver::compare))
		throw InvalidDriverException (name, surname);
	else
		return **driver;
}

std::ostream &
operator << (std::ostream &os, const CDatabase &db)
{
	os << "Drivers:" << endl;
	for (int i = 0; i < db.m_DriversNr; i++)
		db.m_Drivers[i]->print (os, INDENT_CONST);

	os << "Cars:" << endl;
	for (int i = 0; i < db.m_CarsNr; i++)
		db.m_Cars[i]->print (os, INDENT_CONST);

	return os;
}

CDatabase &
CDatabase::operator = (const CDatabase &db)
{
	if (&db == this)
		return *this;

	this->~CDatabase ();
	new (this) CDatabase (db);
	return *this;
}

// --- Test suite --------------------------------------------------------------
#ifndef __PROGTEST__
int
main (int argc, char *argv[])
{
	CDatabase *a, *b, *c;

	a = new CDatabase;
	assert (a->Add ("2012-03-01", "John", "Nowak", "ABC-12-34"));
	assert (a->Add ("2012-03-02", "John", "Nowak", "ABC-12-34"));
	assert (a->Add ("2012-03-03", "John", "Nowak", "DEF-56-67"));
	assert (a->Add ("2012-03-02", "George", "Smith", "DEF-56-67"));

	cout << *a;
	cout << a->FindCar ("ABC-12-34");
	cout << a->FindDriver ("John", "Nowak");

	try {
		cout << a->FindCar ("XYZ-99-88");
		abort ();
	} catch (const InvalidCarException &e) {
		ostringstream ss;
		ss << e;
		assert (ss.str () == "XYZ-99-88");
	}

	try {
		cout << a->FindDriver ("John", "Smith");
		abort ();
	} catch (const InvalidDriverException &e) {
		ostringstream ss;
		ss << e;
		assert (ss.str () == "John Smith");
	}

	delete a;


	a = new CDatabase;
	assert (a->Add ("2012-03-01", "John", "Nowak", "ABC-12-34"));
	assert (a->Add ("2012-03-02", "George", "Smith", "DEF-56-67"));
	cout << *a;
	assert (!a->Add ("2012-03-01", "John", "Nowak", "XYZ-99-99"));
	assert (!a->Add ("2012-03-02", "Peter", "McDonald", "DEF-56-67"));
	cout << *a;

	delete a;


	a = new CDatabase;
	b = new CDatabase;
	assert (a->Add ("2012-03-01", "John", "Nowak", "ABC-12-34"));
	assert (a->Add ("2012-03-02", "John", "Nowak", "DEF-56-67"));
	assert (a->Add ("2012-03-01", "George", "Smith", "DEF-56-67"));
	assert (a->Add ("2012-03-02", "George", "Smith", "ABC-12-34"));
	cout << *a;

	*b = *a;
	c = new CDatabase (*a);
	assert (b->Add ("2012-03-03", "Homer", "Simpson", "ABC-12-34"));
	assert (c->Add ("2012-03-03", "Homer", "Simpson", "DEF-56-67"));
	delete a;
	cout << *b;
	cout << *c;

	delete b;
	delete c;


	CDatabase *db;

	db = new CDatabase;
	const char *names[] = {"A", "B", "C", "D", "E", "F"};
	char buf[40];
	srand (8);
	for (int i = 1000; i < 2000; i++)
	{
		string rz = "ABC-";
		sprintf (buf, "%d", rand () % 100);
		rz += buf;

		string name (names[rand () % 6]);
		string surname (names[rand () % 6]);

		string date = "2012-";
		sprintf (buf, "%d", i);
		date += buf;

		db->Add (date, name, surname, rz);
	}

	ostringstream outa, outb;
	outa << *db;
	outb << CDatabase (*db);
	assert (outa.str () == outb.str ());

	delete db;

	return 0;
}
#endif /* ! __PROGTEST__ */

