#include <p24fxxxx.h>

#define _ADDED_C_LIB 1
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "libves/display.h"
#include "libves/delay.h"
#include "libves/generator.h"
#include "libves/led.h"

// Set the clock to 16MHz etc.
_CONFIG1 (JTAGEN_OFF & GCP_OFF & GWRP_OFF & COE_OFF \
     & FWDTEN_OFF & ICS_PGx2)
_CONFIG2 (0xF7FF & IESO_OFF & FCKSM_CSDCMD & OSCIOFNC_OFF \
     & POSCMOD_HS & FNOSC_PRIPLL & PLLDIV_DIV3 & IOL1WAY_ON)


volatile static unsigned stary_cas, perioda;

void __attribute__((interrupt, auto_psv))
_INT1Interrupt ()
{
	unsigned novy_cas = TMR2;
	perioda = novy_cas - stary_cas;
	stary_cas = novy_cas;

	IFS1bits.INT1IF = 0;     // Vynulovat zadost o preruseni.
}

int
main ()
{
	led_init ();
	disp_init ();
	generator_init ();

	RPINR0bits.INT1R = 17;   // Nastavit preruseni 1 na vstup RP17.

	PR2 = 0xFFFF;            // Preteceni na nejvyssi moznou hodnotu.
	T2CONbits.T32 = 0;       // 16bitovy rezim.
	T2CONbits.TCKPS = 0;     // Delici pomer 1:1.
	T2CONbits.TCS = 0;       // Interni hodiny.
	T2CONbits.TGATE = 0;     // Vypnout Gated Time Accumulation.

	INTCON2bits.INT1EP = 0;  // Pozitivni (nabezna) hrana.

	IFS1bits.INT1IF = 0;     // Vynulovat zadost o preruseni.

	IPC5bits.INT1IP2 = 1;    // Nastavit
	IPC5bits.INT1IP1 = 1;    // prioritu preruseni
	IPC5bits.INT1IP0 = 1;    // na nejvyssi (7).

	stary_cas = 0;

	generator_run (5000);
	TMR2 = 0;                // Vynulovat citac.
	T2CONbits.TON = 1;       // Zapnout casovac T2.
	IEC1bits.INT1IE = 1;     // Povolit preruseni 1.

	for (;;)
	{
		char buf[40];
		unsigned p = perioda;
		disp_clear ();

		disp_at (1, 1);
		snprintf (buf, sizeof buf, "%u tiku", p);
		disp_str (buf);

		disp_at (2, 1);
		snprintf (buf, sizeof buf, "%g ms", p / 16000.);
		disp_str (buf);

		disp_at (3, 1);
		if (p)  snprintf (buf, sizeof buf, "%g Hz", 16000000. / p);
		else    strcpy (buf, "0 Hz");
		disp_str (buf);

		delay_loop_ms (50);
	}

	return 0;
}

