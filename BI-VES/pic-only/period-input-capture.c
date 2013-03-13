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

volatile static unsigned perioda;

void __attribute__((interrupt, auto_psv))
_IC1Interrupt ()
{
	while (IC1CON1bits.ICBNE)
	{
		unsigned prvni = IC1BUF;
		perioda = IC1BUF - prvni;
	}

	IFS0bits.IC1IF = 0;           // Vynulovat zadost o preruseni.
}

int
main ()
{
	led_init ();
	disp_init ();
	generator_init ();

	IPC0bits.IC1IP2 = 1;          // Nastavit
	IPC0bits.IC1IP1 = 1;          // prioritu preruseni
	IPC0bits.IC1IP0 = 1;          // na nejvyssi (7).

	RPINR7bits.IC1R = 17;         // Privest signal na IC1.

	IC1CON2bits.ICTRIG = 0;       // Synchronizace...
	IC1CON2bits.SYNCSEL = 0b1100; // ...s TMR2.
	IC1CON1bits.ICTSEL = 0b001;   // Vybrat TMR2.
	IC1CON1bits.ICI = 0b01;       // Every other capture event.
	IC1CON1bits.ICM = 0b011;      // Capture on every rising edge.

	PR2 = 0xFFFF;                 // Preteceni na nejvyssi moznou hodnotu.
	T2CONbits.T32 = 0;            // 16bitovy rezim.
	T2CONbits.TCKPS = 0;          // Delici pomer 1:1.
	T2CONbits.TCS = 0;            // Interni hodiny.
	T2CONbits.TGATE = 0;          // Vypnout Gated Time Accumulation.

	IFS0bits.IC1IF = 0;           // Vynulovat zadost o preruseni.
	IEC0bits.IC1IE = 1;           // Povolit preruseni od IC1.

	generator_run (5000);
	TMR2 = 0;                     // Vynulovat citac.
	T2CONbits.TON = 1;            // Zapnout casovac T2.

	for (;;)
	{
		char buf[40];
		disp_clear ();

		disp_at (1, 1);
		snprintf (buf, sizeof buf, "%u tiku", perioda);
		disp_str (buf);

		disp_at (2, 1);
		snprintf (buf, sizeof buf, "%g ms", perioda / 16000.);
		disp_str (buf);

		disp_at (3, 1);
		if (perioda)  snprintf (buf, sizeof buf, "%g Hz", 16000000. / perioda);
		else          strcpy (buf, "0 Hz");
		disp_str (buf);

		delay_loop_ms (50);
	}

	return 0;
}

