// FIXME: Not very accurate.

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


volatile static unsigned freq, ctr;

void __attribute__((interrupt, auto_psv))
_T3Interrupt ()
{
	T3CONbits.TON = 0;       // Vypnout casovac T3.

	if (++ctr != 1000)
		goto end;

	ctr = 0;

	T2CONbits.TON = 0;       // Vypnout casovac T2.
	freq = TMR2;             // Priradit pocet pulzu za 1000 ms.
	TMR2 = 0;                // Vynulovat citac.
	T2CONbits.TON = 1;       // Zapnout casovac T2.

end:
	TMR3 = 0;                // Vynulovat citac.
	IFS0bits.T3IF = 0;       // Vynulovat zadost o preruseni.
	T3CONbits.TON = 1;       // Zapnout casovac T3.
}

int
main ()
{
	freq = 0;
	ctr = 0;

	T2CON = 0;
	T3CON = 0;

	led_init ();
	disp_init ();
	generator_init ();

	PR2 = 0xFFFF;            // Preteceni na nejvyssi moznou hodnotu.
	T2CONbits.T32 = 0;       // 16bitovy rezim.
	T2CONbits.TCKPS = 0;     // Delici pomer 1:1.
	T2CONbits.TCS = 1;       // Externi hodiny.
	RPINR3bits.T2CKR = 10;   // Vyber vstupu (RP10).

	PR3 = 15999;             // Kazdou milisekundu -> 1 kHz
	                         // 16 MHz/1 kHz = 16 000
	T3CONbits.TCKPS = 0;     // Delici pomer 1:1.
	T3CONbits.TCS = 0;       // Vnitrni hodiny.
	T3CONbits.TGATE = 0;     // Vypnout Gated Time Accumulation.

	IPC2bits.T3IP2 = 1;      // Nastavit
	IPC2bits.T3IP1 = 1;      // prioritu preruseni
	IPC2bits.T3IP0 = 1;      // na nejvyssi (7).

	IFS0bits.T3IF = 0;       // Vynulovat zadost o preruseni.
	IEC0bits.T3IE = 1;       // Povolit preruseni od T3.

	IFS0bits.T2IF = 0;       // Vynulovat zadost o preruseni.
	IEC0bits.T2IE = 1;       // Povolit preruseni od T2.

	TMR2 = 0;                // Vynulovat
	TMR3 = 0;                // citace.

	double real = generator_run (22050.);    

	T2CONbits.TON = 1;       // Zapnout casovac T2.
	T3CONbits.TON = 1;       // Zapnout casovac T3.

	for (;;)
	{
		char buf[40];

		disp_clear ();
		disp_at (1, 1);
		snprintf (buf, sizeof buf, "%u Hz", freq);
		disp_str (buf);

		disp_at (2, 1);
		snprintf (buf, sizeof buf, "%g Hz", real);
		disp_str (buf);

		delay_loop_ms (100);
	}

	return 0;
}

