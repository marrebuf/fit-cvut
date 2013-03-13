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

int
main ()
{
	disp_init ();

	AD1PCFGLbits.PCFG0 = 0;            // RB0 je analogovy vstup

	AD1CON1bits.FORM   = 0b00;         // unsigned int
	AD1CON1bits.SSRC   = 0b111;        // Konverzi a vzorkovani ridi citac
	AD1CON1bits.ASAM   = 0;            // Vzorkovat pri SAMP = 1

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

	unsigned x = 0;
	while (1)
	{
		AD1CON1bits.SAMP = 1;
		while (!AD1CON1bits.DONE)
			/* busy wait */;

		unsigned int v = ADC1BUF0;

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
