// FIXME: The last time I checked, the queue didn't work 100% correctly.

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

// ----- Queue -----------------------------------------------------------------

struct queue
{
	unsigned char data[256];
	unsigned start;
	unsigned length;
};

static void
queue_init (volatile struct queue *q)
{
	q->start = 0;
	q->length = 0;
}

static inline unsigned
queue_is_empty (volatile struct queue *q)
{
	return q->length == 0;
}

static inline unsigned
queue_is_full (volatile struct queue *q)
{
	return q->length == sizeof q->data;
}

static int
queue_read (volatile struct queue *q)
{
	if (queue_is_empty (q))
		return -1;

	unsigned char c = q->data[q->start++];
	q->length--;
	if (q->start == sizeof q->data)
		q->start = 0;
	return c;
}

static int
queue_write (volatile struct queue *q, unsigned char c)
{
	if (queue_is_full (q))
		return 0;

	unsigned end = q->start + q->length;
	if (end >= sizeof q->data)
		end -= sizeof q->data;
	q->data[end] = c;
	q->length++;
	return 1;
}

// ----- UART API --------------------------------------------------------------

static volatile struct queue g_rx_queue;  // Receive queue.
static volatile struct queue g_tx_queue;  // Transmit queue.

void __attribute__((interrupt, auto_psv))
_U1TXInterrupt ()
{
	if (!queue_is_empty (&g_tx_queue))
		U1TXREG = (char) queue_read (&g_tx_queue);

	IFS0bits.U1TXIF = 0;
}

void __attribute__((interrupt, auto_psv))
_U1RXInterrupt ()
{
	if (U1STAbits.OERR)
		U1STAbits.OERR = 0;
	while (U1STAbits.URXDA)
		if (!queue_is_full (&g_rx_queue))
			queue_write (&g_rx_queue, U1RXREG);

	IFS0bits.U1RXIF = 0;
}

// UART initialization
static void
serial_init (long baudrate)
{
	queue_init (&g_rx_queue);
	queue_init (&g_tx_queue);

	RPINR18bits.U1RXR  = 22;           // UART RX na RP22.
	RPOR12bits.RP24R   = 3;	           // UART TX na RP24.

	U1MODEbits.USIDL   = 0;
	U1MODEbits.IREN    = 0;            // Nechceme IR režim.
	U1MODEbits.UEN1    = 0;            // Pouzivame jen...
	U1MODEbits.UEN0    = 0;            // ...U1RX a U1TX.
	U1MODEbits.WAKE    = 0;
	U1MODEbits.LPBACK  = 0;            // Nechceme loopback režim.
	U1MODEbits.ABAUD   = 0;            // Nechceme autodetekci baudrate.
	U1MODEbits.RXINV   = 0;            // Nechceme invertovane RX.
	U1MODEbits.BRGH    = 0;            // Nechceme high bitrate.
	U1MODEbits.PDSEL   = 0b00;         // 8 bitu, zadna parita.
	U1MODEbits.STSEL   = 0;            // 1 stop bit.

	U1STAbits.UTXISEL1 = 0;            // Preruseni pri kazdem...
	U1STAbits.UTXISEL0 = 0;            // ...odeslanem znaku.
	U1STAbits.URXISEL  = 0b00;         // Preruseni pri kazdem prijatem znaku.
	U1STAbits.ADDEN    = 0;            // Necheme adresy.

	U1BRG = (unsigned) (16000000. / 16 / baudrate) - 1;

	IEC0bits.U1RXIE    = 1;            // Povolit preruseni pro prijem.
	IEC0bits.U1TXIE    = 1;            // Povolit preruseni pro posilani.

	U1MODEbits.UARTEN  = 1;            // Zapnout UART.
	U1STAbits.UTXEN    = 1;            // Zapnout TX.
}

// Read a single character from the input queue
static int
serial_getchar ()
{
	while (1)
	{
		DISABLE_INTERRUPTS
		if (!queue_is_empty (&g_rx_queue))
			break;

		ENABLE_INTERRUPTS
		delay_loop_ms (1);
	}

	int val = queue_read (&g_rx_queue);
	ENABLE_INTERRUPTS
	return val;
}

// Write a single character to the output queue
static void
serial_putchar (char c)
{
	while (1)
	{
		DISABLE_INTERRUPTS
		if (!queue_is_full (&g_tx_queue))
			break;

		ENABLE_INTERRUPTS
		delay_loop_ms (1);
	}

	// Send straight away if we may
	if (queue_is_empty (&g_tx_queue) && !U1STAbits.UTXBF)
		U1TXREG = c;
	else
		queue_write (&g_tx_queue, c);
	ENABLE_INTERRUPTS
}

// Write a string to the output queue
static void
serial_puts (const char *s)
{
	while (*s)
		serial_putchar (*s++);
}

// ----- UART API --------------------------------------------------------------

int
main ()
{
	pl2303_init ();
	led_init ();
	disp_init ();

	serial_init (9600);

	while (1)
	{
		int c;
		switch ((c = serial_getchar ()))
		{
		case '!':
			led_all_off ();
			break;
		case '&':
			switch (serial_getchar ())
			{
			case 'R': led_on  (LED_R); break;
			case 'G': led_on  (LED_G); break;
			case 'B': led_on  (LED_B); break;
			case 'r': led_off (LED_R); break;
			case 'g': led_off (LED_G); break;
			case 'b': led_off (LED_B); break;

			case 'i':
				serial_puts ("Pripravek PIC24f Starter Kit;");
				break;
			case 's':
				disp_clear ();
				disp_at (1, 1);
				while ((c = serial_getchar ()) != ';')
					disp_char (c);
				break;
			default:
				disp_clear ();
				disp_at (1, 1);
				disp_str ("unrecognized '");
				disp_char (c);
				disp_char ('\'');
			}
		}
	}

	return 0;
}
