#include "em_device.h"
#include "em_chip.h"
#include "InitDevice.h"
#include "em_usart.h"
#include "em_gpio.h"
#include "em_emu.h"
#include "segmentlcd.h"
#include <stdio.h>
#include <string.h>

#define END_CHAR (13) // Üzenet vége ASCII karatker, hexában: 0x0013
#define HELP "Help" // Elérhetõ parancsok lekérdezéséhez tartozó üzenet.
#define LED0BE "Set LED 0 1" // LED0 bekapcsolásához tartozó üzenet.
#define LED0KI "Set LED 0 0" // LED0 kikapcsolásához tartozó üzenet.
#define LED1BE "Set LED 1 1" // LED1 kikapcsolásához tartozó üzenet.
#define LED1KI "Set LED 1 0" // LED1 kikapcsolásához tartozó üzenet.
#define GETLED0 "Get LED 0" // LED0 lekérdezéséhez tartozó üzenet.
#define GETLED1 "Get LED 1" // LED1 lekérdezéséhez tartozó üzenet.
#define WRITETEXT "Write Text" // Kijelzõn futó szöveghez tartozó üzenet.
#define WRITETEXT_LENGTH (10 + 1)

// A PC-rõl UART-on érkezett üzenet.
char message[100 + 1];
int messageSize = 0;
char command[50 + 1];

uint8_t /*volatile*/ ch;
bool volatile new_char = false;

// Változó egy új üzenet jelzésére. Az értéke true, ha új (még feldolgozatlan) üzenet érkezett.
bool volatile receivedMessage = false;

// Write Text parancshoz flag.
// Értéke true, ha éppen futó szöveg fut a kijelzõn. Egyébként false az értéke.
bool volatile writingText = false;

void UART0_RX_IRQHandler(void) {
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

int main(void)
{
	// Chip errata
	CHIP_Init();

	// Init device using Configurator
	enter_DefaultMode_from_RESET();

	// Üzenet változó alaphelyzetbe állítása: üres üzenet.
	message[0] = '\0';
	messageSize = 0;

	// Init device (additional settings, not available in Configurator)
	USART_IntEnable(UART0, USART_IF_RXDATAV);
	NVIC_EnableIRQ(UART0_RX_IRQn);

	// Init LEDs
	GPIO_PinOutClear(LED0_PORT, LED0_PIN);
	GPIO_PinOutClear(LED1_PORT, LED1_PIN);
	SegmentLCD_Init(false);

	// Üdvözlõ üzenet a kijelzõn.
	SegmentLCD_Write("CLI");

	// Infinite loop
	while (1)
	{
		EMU_EnterEM1();

		if (new_char) {
			new_char = false;
			// USART_Tx(UART0, ch);
			//GPIO_PinOutToggle(LED0_PORT, LED0_PIN);
			//GPIO_PinOutToggle(LED1_PORT, LED1_PIN);
			SegmentLCD_Number(ch);
		}

		// Ha új üzennet érkezett, akkor dolgozzuk fel.
		if(receivedMessage)
		{
			receivedMessage = false; // Feldolgozás után új üzenet várunk majd.

			// Üzenetek feldolgozása:
			if(strcmp(message, HELP) == 0)
			{
				string2USART(HELP); USART_Tx(UART0, '\n');
				string2USART(LED0BE); USART_Tx(UART0, '\n');
				string2USART(LED0KI); USART_Tx(UART0, '\n');
				string2USART(LED1BE); USART_Tx(UART0, '\n');
				string2USART(LED1KI); USART_Tx(UART0, '\n');
				string2USART(GETLED0); USART_Tx(UART0, '\n');
				string2USART(GETLED1);
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
				if(value == 1)
				{
					string2USART("LED0 is light");
					SegmentLCD_Write("LED0 1");
				}
				else
				{
					string2USART("LED0 is dark");
					SegmentLCD_Write("LED0 0");
				}
				SegmentLCD_Number(value);
			}
			else if(strcmp(message, GETLED1) == 0) // Ha az üzenet a LED1 lekérdezése, akkor ...
			{
				int value = GPIO_PinOutGet(LED1_PORT, LED1_PIN); // kérdezzük le a LED1-et.
				if(value == 1)
				{
					string2USART("LED1 is light");
					SegmentLCD_Write("LED1 1");
				}
				else
				{
					string2USART("LED1 is dark");
					SegmentLCD_Write("LED1 0");
				}
				SegmentLCD_Number(value);
			}
			else
			{
				if(strlen(message) > WRITETEXT_LENGTH)
				{
					int i;
					int j;
					// int length = strlen(message);
					for(j = 0, i = WRITETEXT_LENGTH; message[i] != '\0'; i++, j++)
					{
						command[j] = message[i];
					}
					command[j] = '\0';
					writingText = true;
				}
				else
				{
					string2USART("Invalid command. :-(");
					SegmentLCD_Write("8-(");
				}
			}

			if(writingText)
			{
				SegmentLCD_Write(command);
			}

			echoMessage();

			string2USART(">>");

			// Üzenetet feldolgoztuk, "töröljük" az elõzõ üzenetet az új fogadása elõtt.
			message[0] = '\0';
			messageSize = 0;
		}
	}
}
