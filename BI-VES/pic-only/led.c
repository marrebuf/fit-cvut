#include <p24fxxxx.h>
#include "delay_loop.h"

// Set the clock to 16MHz etc.
_CONFIG1 (JTAGEN_OFF & GCP_OFF & GWRP_OFF & COE_OFF \
     & FWDTEN_OFF & ICS_PGx2)
_CONFIG2 (0xF7FF & IESO_OFF & FCKSM_CSDCMD & OSCIOFNC_OFF \
     & POSCMOD_HS & FNOSC_PRIPLL & PLLDIV_DIV3 & IOL1WAY_ON)

#define LED_R 1
#define LED_G 2
#define LED_B 4

static void
led (int led_id, int on)
{
	switch (led_id)
	{
	case LED_R:
		if (on)  LATF &= ~((1 << 4) | (1 << 5));
		else     LATF |=   (1 << 4) | (1 << 5);
		break;
	case LED_G:
		if (on)  LATG &= ~((1 << 8) | (1 << 9));
		else     LATG |=   (1 << 8) | (1 << 9);
		break;
	case LED_B:
		if (on)  LATG &= ~((1 << 6) | (1 << 7));
		else     LATG |=   (1 << 6) | (1 << 7);
		break;
	}
}

static void
wait ()
{
	volatile unsigned int i;
	for (i = 0; i < 30000; i++)
		delay_loop ();
}

int
main ()
{
	// LED init

	// LED jako vystupy
	TRISF &= ~((1 << 4) | (1 << 5));
	TRISG &= ~((1 << 6) | (1 << 7) | (1 << 8) | (1 << 9));

	// LED jako open-drain vystupy
	ODCF  |=   (1 << 4) | (1 << 5);
	ODCG  |=   (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9);

	// Zhasnout vsechny LED
	LATF  |=   (1 << 4) | (1 << 5);
	LATG  |=   (1 << 6) | (1 << 7) | (1 << 8) | (1 << 9);

	while (1)
	{
		led (LED_R, 1);  // Cervena
		wait ();
		led (LED_G, 1);  // Cervena + Zelena = Zluta
		wait ();
		led (LED_R, 0);  // Zelena
		wait ();
		led (LED_R, 1);  // Zelena + Cervena = Zluta
		wait ();
		led (LED_G, 0);  // Cervena
	}

	return 0;
}

