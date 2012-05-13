#ifndef __AUTA_H__
#define __AUTA_H__

class Auto
{
protected:
	std::string spz;
	int rok_vyroby;
public:
	Auto (std::string _spz, int _rok_vyroby)
		: spz (_spz), rok_vyroby (_rok_vyroby) {}
	std::string get_spz () {return spz;}
	int get_rok_vyroby () {return rok_vyroby;}

	virtual void print (std::ostream &os)
	{
		os << spz << " r" << rok_vyroby;
	}
};

class Osobni : public Auto
{
protected:
	int pocet_osob;
public:
	Osobni (std::string _spz, int _rok_vyroby, int _pocet_osob)
		: Auto (_spz, _rok_vyroby), pocet_osob (_pocet_osob) {}

	virtual void print (std::ostream &os)
	{
		os << spz << " r" << rok_vyroby << ' ' << pocet_osob << ' ';
		switch (pocet_osob)
		{
		case 1:
			os << "osoba";
			break;
		case 2:
		case 3:
		case 4:
			os << "osoby";
			break;
		default:
			os << "osob";
			break;
		}
	}
};

class Nakladni : public Auto
{
protected:
	int nosnost;
public:
	Nakladni (std::string _spz, int _rok_vyroby, int _nosnost)
		: Auto (_spz, _rok_vyroby), nosnost (_nosnost) {}

	virtual void print (std::ostream &os)
	{
		os << spz << " r" << rok_vyroby << ' ' << nosnost << " kg";
	}
};

#endif /* ! __AUTA_H__ */

