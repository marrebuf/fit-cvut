#include <p24fxxxx.h>

#include "libves/delay.h"

#include "touch.h"

void
touch_init ()
{
	// Copied over from touch_1250.pdf

	// CTMUCON
	CTMUCONbits.CTMUEN   = 0;       // Make sure CTMU is disabled
	CTMUCONbits.CTMUSIDL = 0;       // CTMU continues to run in idle mode
	CTMUCONbits.TGEN     = 0;       // Disable edge delay gen. mode of the CTMU
	CTMUCONbits.EDGEN    = 0;       // Edges are blocked
	CTMUCONbits.EDGSEQEN = 0;       // Edge sequence not needed
	CTMUCONbits.IDISSEN  = 0;       // Do not ground the current source
	CTMUCONbits.CTTRIG   = 0;       // Trigger Output is disabled
	CTMUCONbits.EDG2POL  = 0;
	CTMUCONbits.EDG2SEL  = 0x3;     // Edge2 Src = OC1 (don't care)
	CTMUCONbits.EDG1POL  = 1;
	CTMUCONbits.EDG1SEL  = 0x3;     // Edge1 Src = Timer1 (don't care)

	// CTMUICON
	CTMUICON             = 0x300;   // 55uA
	CTMUICONbits.ITRIM   = 0;       // Nominal - No Adjustment

	// Setup A/D converter
	AD1PCFGL             = 0x0000;
	AD1CON1              = 0x0000;
	AD1CHS               = 0x0000;  // Select the analog channel 0
	AD1CSSL              = 0x0000;
	AD1CON1bits.FORM     = 0x0;     // Unsigned fractional format
	AD1CON3              = 0x0000;  // bits.ADRC = 0;
	AD1CON2              = 0x0000;
	AD1CON1bits.ADON     = 1;       // Turn On A/D
	CTMUCONbits.CTMUEN   = 1;       // Enable CTMU
}	

unsigned
touch_sample (int ch)
{
	// Similar, although not exactly line for line

	AD1PCFGLbits.PCFG8   = 1;       // RB8  je digitalni vystup
	AD1PCFGLbits.PCFG9   = 1;       // RB9  je digitalni vystup
	AD1PCFGLbits.PCFG10  = 1;       // RB10 je digitalni vystup
	AD1PCFGLbits.PCFG11  = 1;       // RB11 je digitalni vystup
	AD1PCFGLbits.PCFG12  = 1;       // RB12 je digitalni vystup

	TRISBbits.TRISB8     = 0;
	TRISBbits.TRISB9     = 0;
	TRISBbits.TRISB10    = 0;
	TRISBbits.TRISB11    = 0;
	TRISBbits.TRISB12    = 0;

	LATBbits.LATB8       = 0;
	LATBbits.LATB9       = 0;
	LATBbits.LATB10      = 0;
	LATBbits.LATB11      = 0;
	LATBbits.LATB12      = 0;

	PORTBbits.RB8        = 0;
	PORTBbits.RB9        = 0;
	PORTBbits.RB10       = 0;
	PORTBbits.RB11       = 0;
	PORTBbits.RB12       = 0;

	Nop (); Nop (); Nop (); Nop (); Nop (); Nop (); Nop (); Nop ();

	TRISBbits.TRISB8     = 1;
	TRISBbits.TRISB9     = 1;
	TRISBbits.TRISB10    = 1;
	TRISBbits.TRISB11    = 1;
	TRISBbits.TRISB12    = 1;

	AD1PCFGLbits.PCFG8   = 0;       // RB8  je analogovy vstup
	AD1PCFGLbits.PCFG9   = 0;       // RB9  je analogovy vstup
	AD1PCFGLbits.PCFG10  = 0;       // RB10 je analogovy vstup
	AD1PCFGLbits.PCFG11  = 0;       // RB11 je analogovy vstup
	AD1PCFGLbits.PCFG12  = 0;       // RB12 je analogovy vstup

	Nop (); Nop (); Nop (); Nop (); Nop (); Nop (); Nop (); Nop ();

	AD1CHSbits.CH0SA     = ch;

	CTMUCONbits.IDISSEN  = 1;       // Odstranit naboj
	Nop (); Nop (); Nop (); Nop (); Nop (); Nop (); Nop (); Nop ();
	CTMUCONbits.IDISSEN  = 0;       // Konec discharge

	AD1CON1bits.SAMP     = 1;       // Zacit vzorkovani
	CTMUCONbits.EDG2STAT = 0;       // Zapnout zdroj
	CTMUCONbits.EDG1STAT = 1;       // konstantniho proudu

	int i;
	for (i = 0; i < 1; i++)
		delay_loop_1us ();

	AD1CON1bits.SAMP     = 0;       // Ukoncit vzorkovani

	while (!AD1CON1bits.DONE)
		/* busy wait */;

	unsigned int v = ADC1BUF0;
	AD1CON1bits.DONE     = 0;
	return v;
}

unsigned
touch_pressed (int channel)
{
	const int thres = 550;

	int a = touch_sample (channel) < thres;
	delay_loop_ms (3);
	int b = touch_sample (channel) < thres;
	delay_loop_ms (3);
	int c = touch_sample (channel) < thres;

	return (a && c) || (a && b) || (b && c);
}

int
touch_getkey ()
{
	if (touch_pressed (8))  return 1;
	if (touch_pressed (9))  return 2;
	if (touch_pressed (10)) return 3;
	if (touch_pressed (11)) return 4;
	if (touch_pressed (12)) return 5;
	return 0;
}

int
touch_readkey ()
{
	int key;
	do
		key = touch_getkey ();
	while (key == 0);

	while (touch_getkey () != 0)
		/* busy wait */;
	return key;
}

