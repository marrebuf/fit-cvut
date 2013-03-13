#include <p24fxxxx.h>

#define _ADDED_C_LIB 1
#include <stdio.h>
#include <string.h>

#include "libves/display.h"
#include "libves/delay.h"
#include "libves/generator.h"
#include "libves/led.h"
#include "libves/pl2303.h"

// Set the clock to 16MHz etc.
_CONFIG1 (JTAGEN_OFF & GCP_OFF & GWRP_OFF & COE_OFF \
     & FWDTEN_OFF & ICS_PGx2)
_CONFIG2 (0xF7FF & IESO_OFF & FCKSM_CSDCMD & OSCIOFNC_OFF \
     & POSCMOD_HS & FNOSC_PRIPLL & PLLDIV_DIV3 & IOL1WAY_ON)

#define DISABLE_INTERRUPTS  __asm__ volatile ("disi #0x3FFF");
#define ENABLE_INTERRUPTS   __asm__ volatile ("disi #0000");

static volatile unsigned int g_value, g_updated;

void __attribute__((interrupt, auto_psv))
_ADC1Interrupt ()
{
	g_value = ADC1BUF0;                // Precist zkonvertovanou hodnotu
	g_updated = 1;                     // Oznamit zmenu hlavnimu programu
	IFS0bits.AD1IF = 0;                // Sundat interrupt flag
}

int
main ()
{
	disp_init ();

	AD1PCFGLbits.PCFG0 = 0;            // RB0 je analogovy vstup

	AD1CON1bits.FORM   = 0b00;         // unsigned int
	AD1CON1bits.SSRC   = 0b010;        // Konverzi a vzorkovani ridi TMR3
	AD1CON1bits.ASAM   = 1;            // Vzorkovat po skonceni konverze

	AD1CON2bits.VCFG   = 0b000;        // VR+ AVdd, VR- AVss
	AD1CON2bits.CSCNA  = 0;            // MUX A
	AD1CON2bits.SMPI   = 0;            // Interrupt pro kazdou konverzi
	AD1CON2bits.BUFM   = 0;            // 16bit buffer
	AD1CON2bits.ALTS   = 0;            // Jen MUX A

	AD1CON3bits.ADRC   = 0;            // Clock source -> System clock
	AD1CON3bits.SAMC   = 1;            // Konverzni cas v jednotkach Tad
	AD1CON3bits.ADCS   = 1;            // Pro F = 16 MHz je T = 62.5 ns, chceme >75

	AD1CHSbits.CH0NA   = 0;            // Channel 0 - = Vr-
	AD1CHSbits.CH0SA   = 0b00000;      // Channel 0 + = AN0

	AD1CON1bits.ADON   = 1;            // Zapnout prevodnik

	IEC0bits.AD1IE     = 1;            // Povolit preruseni pro ADC.

	PR3 = 19999;                       // Kazdych 10 ms -> 100 Hz
	                                   // 16 MHz/100 Hz = 160 000 / 8 = 20 000
	T3CONbits.TCKPS = 0b01;            // Delici pomer 1:8
	T3CONbits.TCS = 0;                 // Vnitrni hodiny.
	T3CONbits.TGATE = 0;               // Vypnout Gated Time Accumulation.

	T3CONbits.TON = 1;                 // Zapnout casovac T3.

	AD1CON1bits.SAMP = 1;

	unsigned x = 0;
	while (1)
	{
		while (!g_updated)
			/* busy wait */;

		DISABLE_INTERRUPTS
		unsigned int v = g_value;
		g_updated = 0;
		ENABLE_INTERRUPTS

		char buf[40];

		disp_at (1, 1);
		snprintf (buf, sizeof buf, "%g V ", v / 1024. * 3.3);
		disp_str (buf);

		set_color (0);
		draw_line (x, 0, x, 64);
		put_pixel (x, v >> 4, COLOR_WHITE);

		x = (x + 1) & 127;
		delay_loop_ms (10);
	}

	return 0;
}
