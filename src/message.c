#include <stdbool.h> // bool, true, false
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "em_usart.h"
#include "em_timer.h"
#include "em_gpio.h"
#include "InitDevice.h"
#include "message.h" // Üzenetkezeléshez tartozó változók és függvények.
#include "constants.h" // END_CHAR
#include "segmentlcd.h"

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

void processCommand(char * message)
{
	if(strcmp(message, HELP) == 0)
	{
		parancsok();
	}
	else if(strcmp(message, LED0BE) == 0) // Ha az üzenet a LED0 bekapcsolása, akkor ...
	{
		GPIO_PinOutSet(LED0_PORT, LED0_PIN); // kapcsoljuk be a LED0-et.
	}
	else if(strcmp(message, LED0KI) == 0) // Ha az üzenet a LED0 kikapcsolása, akkor ...
	{
		GPIO_PinOutClear(LED0_PORT, LED0_PIN); // kapcsoljuk ki a LED0-et.
	}
	else if(strcmp(message, LED1BE) == 0) // Ha az üzenet a LED1 bekapcsolása, akkor ...
	{
		GPIO_PinOutSet(LED1_PORT, LED1_PIN); // kapcsoljuk be a LED1-et.
	}
	else if(strcmp(message, LED1KI) == 0) // Ha az üzenet a LED1 kikapcsolása, akkor ...
	{
		GPIO_PinOutClear(LED1_PORT, LED1_PIN); // kapcsoljuk ki a LED1-et.
	}
	else if(strcmp(message, GETLED0) == 0) // Ha az üzenet a LED0 lekérdezése, akkor ...
	{
		int value = GPIO_PinOutGet(LED0_PORT, LED0_PIN); // kérdezzük le a LED0-et.
		USART_Tx(UART0, '\n');
		if(value == 1)
		{
			string2USART("LED0 vilagit");
			SegmentLCD_Write("LED0 1");
		}
		else
		{
			string2USART("LED0 nem vilagit");
			SegmentLCD_Write("LED0 0");
		}
		SegmentLCD_Number(value);
	}
	else if(strcmp(message, GETLED1) == 0) // Ha az üzenet a LED1 lekérdezése, akkor ...
	{
		int value = GPIO_PinOutGet(LED1_PORT, LED1_PIN); // kérdezzük le a LED1-et.
		USART_Tx(UART0, '\n');
		if(value == 1)
		{
			string2USART("LED1 vilagit");
			SegmentLCD_Write("LED1 1");
		}
		else
		{
			string2USART("LED1 nem vilagit");
			SegmentLCD_Write("LED1 0");
		}
		SegmentLCD_Number(value);
	}
	else // Invalid command vagy Write Text, ha a fentiek egyike se
	{
		// Write Text az üzenet?
		if(strlen(message) > WRITETEXT_LENGTH)
		{
			int i;
			int j;
			// Üzenet elejét bemásoljuk egy segédtömbbe.
			for(j = 0, i = 0; i < (WRITETEXT_LENGTH-1); i++, j++)
			{
				command[j] = message[i];
			}
			command[j] = '\0';
			// Üzenet eleje megegyezik a WRITETEXT szöveggel?
			if(strcmp(command, WRITETEXT) == 0)
			{
				// Ha igen, akkor Write Text parancs érkezett és üzenet kijelzése az LCD-re.
				for(j = 0, i = WRITETEXT_LENGTH; message[i] != '\0'; i++, j++)
				{
					command[j] = message[i];
				}
				command[j] = '\0';
				USART_Tx(UART0, '\n');
				string2USART(command);
				writingText = true;
				step = 0;
			}
		}
		// Ha nem, akkor Invalid command.
		else
		{
			string2USART(INVALID);
			SegmentLCD_Write("8-("); // Szomorú fej az LCD-re. :-(
			// A ':' karakter nem szép az LCD-n, ezért napszemüveges szomorú fej: 8-(
		}
	}
}

void TIMER0_IRQHandler(void)
{
  ms_counter++;                         // Increment counter
  TIMER_IntClear(TIMER0, TIMER_IF_OF);  // Clear overflow flag
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
