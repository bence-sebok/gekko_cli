#include <stdbool.h> // bool, true, false
#include "em_usart.h"
#include "em_timer.h"
#include "message.h" // Üzenetkezeléshez tartozó változók és függvények.
#include "constants.h" // END_CHAR
#include "segmentlcd.h"
#include <stdlib.h>

// A PC-rõl UART-on keresztül érkezett üzenet.
char message[100 + 1] = ""; // Üzenet tartalma.
int messageSize = 0; // Üzenet hossza.
char command[50 + 1]; //
int step = 0; // Hányadik 7 darabos karaktersorozatot jelenítsük meg.

// Változó egy új üzenet jelzésére. Az értéke true, ha új (még feldolgozatlan) üzenet érkezett.
bool volatile receivedMessage = false;

// Write Text parancshoz flag. Értéke true, ha éppen futó szöveg fut a kijelzõn. Egyébként false az értéke.
bool volatile writingText = false;
char screen[KIJELZO_MERET + 1]; // Ennek a tartalma kerül majd az LCD-re.

uint8_t ch; // UART-on kapott karakter.
bool volatile new_char = false; // Érkezett-e új karakter flag.

uint16_t ms_counter = 0; // Milliszekundumos iterációhoz.

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

// Teljes üzenet kiírása a PC-s terminálra UART-on keresztül.
void echoMessage()
{
	// Formátum: ÚJ SOR<ÜZENET>
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

	// Ha az érkezett karakter az üzenet vége jel, akkor vége egy üzenetnek és dolgozzuk fel (feldolgozás flag set)
	if(ch == END_CHAR)
	{
		message[messageSize++] = '\0';
		receivedMessage = true; // Flag beállítása, hogy a main-ben feldolgozzuk az üzenetet.
	}
	// Ha még nem jött az üzenet vége jel, akkor tároljuk el az új karaktert az üzenetben.
	else
	{
		// A messageSize változó mindig az új karakter indexelésére alkalmas.
		message[messageSize] = ch;
		messageSize++; // Jött egy új karakter, növeljük az üzenet hosszát jelentõ változót.
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
	memcpy(screen, string + step, KIJELZO_MERET); // step-edik 7 darab karakter másolása az LCD-re.
	screen[KIJELZO_MERET] = '\0'; // Lezárás a sztringgé alakítás miatt.
	SegmentLCD_Write(screen);
}
