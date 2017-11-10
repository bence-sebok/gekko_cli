#include "em_device.h"
#include "em_chip.h"
#include "InitDevice.h"
#include "em_usart.h"
#include "em_gpio.h"
#include "em_emu.h"
#include "segmentlcd.h"
#include <stdio.h>
#include <string.h>

#define END_CHAR (13) // �zenet v�ge ASCII karatker, hex�ban: 0x0013
#define HELP "Help" // El�rhet� parancsok lek�rdez�s�hez tartoz� �zenet.
#define LED0BE "Set LED 0 1" // LED0 bekapcsol�s�hoz tartoz� �zenet.
#define LED0KI "Set LED 0 0" // LED0 kikapcsol�s�hoz tartoz� �zenet.
#define LED1BE "Set LED 1 1" // LED1 kikapcsol�s�hoz tartoz� �zenet.
#define LED1KI "Set LED 1 0" // LED1 kikapcsol�s�hoz tartoz� �zenet.
#define GETLED0 "Get LED 0" // LED0 lek�rdez�s�hez tartoz� �zenet.
#define GETLED1 "Get LED 1" // LED1 lek�rdez�s�hez tartoz� �zenet.
#define WRITETEXT "Write Text" // Kijelz�n fut� sz�veghez tartoz� �zenet.
#define WRITETEXT_LENGTH (10 + 1)

// A PC-r�l UART-on �rkezett �zenet.
char message[100 + 1];
int messageSize = 0;
char command[50 + 1];

uint8_t /*volatile*/ ch;
bool volatile new_char = false;

// V�ltoz� egy �j �zenet jelz�s�re. Az �rt�ke true, ha �j (m�g feldolgozatlan) �zenet �rkezett.
bool volatile receivedMessage = false;

// Write Text parancshoz flag.
// �rt�ke true, ha �ppen fut� sz�veg fut a kijelz�n. Egy�bk�nt false az �rt�ke.
bool volatile writingText = false;

void UART0_RX_IRQHandler(void) {
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

int main(void)
{
	// Chip errata
	CHIP_Init();

	// Init device using Configurator
	enter_DefaultMode_from_RESET();

	// �zenet v�ltoz� alaphelyzetbe �ll�t�sa: �res �zenet.
	message[0] = '\0';
	messageSize = 0;

	// Init device (additional settings, not available in Configurator)
	USART_IntEnable(UART0, USART_IF_RXDATAV);
	NVIC_EnableIRQ(UART0_RX_IRQn);

	// Init LEDs
	GPIO_PinOutClear(LED0_PORT, LED0_PIN);
	GPIO_PinOutClear(LED1_PORT, LED1_PIN);
	SegmentLCD_Init(false);

	// �dv�zl� �zenet a kijelz�n.
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

		// Ha �j �zennet �rkezett, akkor dolgozzuk fel.
		if(receivedMessage)
		{
			receivedMessage = false; // Feldolgoz�s ut�n �j �zenet v�runk majd.

			// �zenetek feldolgoz�sa:
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
			else if(strcmp(message, GETLED1) == 0) // Ha az �zenet a LED1 lek�rdez�se, akkor ...
			{
				int value = GPIO_PinOutGet(LED1_PORT, LED1_PIN); // k�rdezz�k le a LED1-et.
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

			// �zenetet feldolgoztuk, "t�r�lj�k" az el�z� �zenetet az �j fogad�sa el�tt.
			message[0] = '\0';
			messageSize = 0;
		}
	}
}
