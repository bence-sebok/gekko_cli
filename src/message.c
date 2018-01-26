#include <stdbool.h> // bool, true, false
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "em_usart.h"
#include "em_timer.h"
#include "em_gpio.h"
#include "InitDevice.h"
#include "message.h" // �zenetkezel�shez tartoz� v�ltoz�k �s f�ggv�nyek.
#include "constants.h" // END_CHAR
#include "segmentlcd.h"

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

void processCommand(char * message)
{
	if(strcmp(message, HELP) == 0)
	{
		parancsok();
	}
	else if(strcmp(message, LED0BE) == 0) // Ha az �zenet a LED0 bekapcsol�sa, akkor ...
	{
		GPIO_PinOutSet(LED0_PORT, LED0_PIN); // kapcsoljuk be a LED0-et.
	}
	else if(strcmp(message, LED0KI) == 0) // Ha az �zenet a LED0 kikapcsol�sa, akkor ...
	{
		GPIO_PinOutClear(LED0_PORT, LED0_PIN); // kapcsoljuk ki a LED0-et.
	}
	else if(strcmp(message, LED1BE) == 0) // Ha az �zenet a LED1 bekapcsol�sa, akkor ...
	{
		GPIO_PinOutSet(LED1_PORT, LED1_PIN); // kapcsoljuk be a LED1-et.
	}
	else if(strcmp(message, LED1KI) == 0) // Ha az �zenet a LED1 kikapcsol�sa, akkor ...
	{
		GPIO_PinOutClear(LED1_PORT, LED1_PIN); // kapcsoljuk ki a LED1-et.
	}
	else if(strcmp(message, GETLED0) == 0) // Ha az �zenet a LED0 lek�rdez�se, akkor ...
	{
		int value = GPIO_PinOutGet(LED0_PORT, LED0_PIN); // k�rdezz�k le a LED0-et.
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
	else if(strcmp(message, GETLED1) == 0) // Ha az �zenet a LED1 lek�rdez�se, akkor ...
	{
		int value = GPIO_PinOutGet(LED1_PORT, LED1_PIN); // k�rdezz�k le a LED1-et.
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
		// Write Text az �zenet?
		if(strlen(message) > WRITETEXT_LENGTH)
		{
			int i;
			int j;
			// �zenet elej�t bem�soljuk egy seg�dt�mbbe.
			for(j = 0, i = 0; i < (WRITETEXT_LENGTH-1); i++, j++)
			{
				command[j] = message[i];
			}
			command[j] = '\0';
			// �zenet eleje megegyezik a WRITETEXT sz�veggel?
			if(strcmp(command, WRITETEXT) == 0)
			{
				// Ha igen, akkor Write Text parancs �rkezett �s �zenet kijelz�se az LCD-re.
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
			SegmentLCD_Write("8-("); // Szomor� fej az LCD-re. :-(
			// A ':' karakter nem sz�p az LCD-n, ez�rt napszem�veges szomor� fej: 8-(
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
