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
#define LED1BE "Set LED 1 1" // LED1 bekapcsol�s�hoz tartoz� �zenet.
#define LED1KI "Set LED 1 0" // LED1 kikapcsol�s�hoz tartoz� �zenet.

// A PC-r�l UART-on �rkezett �zenet.
char message[100];
int messageSize = 0;

uint8_t /*volatile*/ ch;
bool volatile new_char = false;

// V�ltoz� egy �j �zenet jelz�s�re. Az �rt�ke true, ha �j (m�g feldolgozatlan) �zenet �rkezett.
bool volatile receivedMessage = false;

void UART0_RX_IRQHandler(void) {
	ch = USART_RxDataGet(UART0);
	new_char = true;

	// Ha az �rkezett karakter az �zenet v�ge jel, akkor v�ge egy �zenetnek �s dolgozzuk fel (feldolgoz�s flag set)
	if(ch == END_CHAR)
	{
		SegmentLCD_Write("msgEnd");
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

int main(void)
{
	int i; // Sok iter�ci�hoz kell.

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

	// Init board
	GPIO_PinOutSet(LED0_PORT, LED0_PIN);
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
			if(strcmp(message, LED1BE) == 0) // Ha az �zenet a LED1 bekapcsol�sa, akkor ...
			{
				GPIO_PinOutSet(LED1_PORT, LED1_PIN); // kapcsoljuk be a LED1-et.
			}
			else if(strcmp(message, LED1KI) == 0) // Ha az �zenet a LED1 kikapcsol�sa, akkor ...
			{
				GPIO_PinOutClear(LED1_PORT, LED1_PIN); // kapcsoljuk ki a LED1-et.
			}
			SegmentLCD_Write("newMsg");
			// Teljes �zenet ki�r�sa a PC-s termin�lra UART-on kereszt�l.
			USART_Tx(UART0, '<');
			for(i = 0; i < messageSize; i++)
			{
				USART_Tx(UART0, message[i]);
			}
			USART_Tx(UART0, '>');

			// �zenetet feldolgoztuk, "t�r�lj�k" az el�z� �zenetet az �j fogad�sa el�tt.
			message[0] = '\0';
			messageSize = 0;
		}
	}
}
