#include <stdbool.h> // bool, true, false
#include "em_usart.h"
#include "em_timer.h"
#include "message.h" // �zenetkezel�shez tartoz� v�ltoz�k �s f�ggv�nyek.
#include "constants.h" // END_CHAR
#include "segmentlcd.h"
#include <stdlib.h>

// A PC-r�l UART-on kereszt�l �rkezett �zenet.
char message[100 + 1] = ""; // �zenet tartalma.
int messageSize = 0; // �zenet hossza.
char command[50 + 1]; //
int step = 0; // H�nyadik 7 darabos karaktersorozatot jelen�ts�k meg.

// V�ltoz� egy �j �zenet jelz�s�re. Az �rt�ke true, ha �j (m�g feldolgozatlan) �zenet �rkezett.
bool volatile receivedMessage = false;

// Write Text parancshoz flag. �rt�ke true, ha �ppen fut� sz�veg fut a kijelz�n. Egy�bk�nt false az �rt�ke.
bool volatile writingText = false;
char screen[KIJELZO_MERET + 1]; // Ennek a tartalma ker�l majd az LCD-re.

uint8_t ch; // UART-on kapott karakter.
bool volatile new_char = false; // �rkezett-e �j karakter flag.

uint16_t ms_counter = 0; // Milliszekundumos iter�ci�hoz.

void TIMER0_IRQHandler(void)
{
  ms_counter++;                           // Increment counter
  TIMER_IntClear(TIMER0, TIMER_IF_OF);      // Clear overflow flag
}

void string2USART(char * string)
{
	int i;
	for(i = 0; string[i] != '\0'; i++)
	{
			USART_Tx(UART0, string[i]);
	}
}

// Teljes �zenet ki�r�sa a PC-s termin�lra UART-on kereszt�l.
void echoMessage()
{
	// Form�tum: �J SOR<�ZENET>
	USART_Tx(UART0, '\n');
	string2USART("Echo:");
	USART_Tx(UART0, '<');
	string2USART(message);
	USART_Tx(UART0, '>');
	USART_Tx(UART0, '\n');
}

void UART0_RX_IRQHandler(void)
{
	ch = USART_RxDataGet(UART0);
	new_char = true;

	// Ha az �rkezett karakter az �zenet v�ge jel, akkor v�ge egy �zenetnek �s dolgozzuk fel (feldolgoz�s flag set)
	if(ch == END_CHAR)
	{
		message[messageSize++] = '\0';
		receivedMessage = true; // Flag be�ll�t�sa, hogy a main-ben feldolgozzuk az �zenetet.
	}
	// Ha m�g nem j�tt az �zenet v�ge jel, akkor t�roljuk el az �j karaktert az �zenetben.
	else
	{
		// A messageSize v�ltoz� mindig az �j karakter indexel�s�re alkalmas.
		message[messageSize] = ch;
		messageSize++; // J�tt egy �j karakter, n�velj�k az �zenet hossz�t jelent� v�ltoz�t.
	}
	//USART_IntClear(UART0, USART_IF_RXDATAV);
}

void parancsok(void)
{
	USART_Tx(UART0, '\n');
	string2USART("Parancsok:"); USART_Tx(UART0, '\n');
	string2USART(WRITETEXT); USART_Tx(UART0, '\n');
	string2USART(HELP); USART_Tx(UART0, '\n');
	string2USART(LED0BE); USART_Tx(UART0, '\n');
	string2USART(LED0KI); USART_Tx(UART0, '\n');
	string2USART(LED1BE); USART_Tx(UART0, '\n');
	string2USART(LED1KI); USART_Tx(UART0, '\n');
	string2USART(GETLED0); USART_Tx(UART0, '\n');
	string2USART(GETLED1);
}

void updateScreen(char * screen, char * string)
{
	memcpy(screen, string + step, KIJELZO_MERET); // step-edik 7 darab karakter m�sol�sa az LCD-re.
	screen[KIJELZO_MERET] = '\0'; // Lez�r�s a sztringg� alak�t�s miatt.
	SegmentLCD_Write(screen);
}
