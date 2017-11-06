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
#define LED1BE "Set LED 1 1" // LED1 bekapcsolásához tartozó üzenet.
#define LED1KI "Set LED 1 0" // LED1 kikapcsolásához tartozó üzenet.

// A PC-rõl UART-on érkezett üzenet.
char message[100];
int messageSize = 0;

uint8_t /*volatile*/ ch;
bool volatile new_char = false;

// Változó egy új üzenet jelzésére. Az értéke true, ha új (még feldolgozatlan) üzenet érkezett.
bool volatile receivedMessage = false;

void UART0_RX_IRQHandler(void) {
	ch = USART_RxDataGet(UART0);
	new_char = true;

	// Ha az érkezett karakter az üzenet vége jel, akkor vége egy üzenetnek és dolgozzuk fel (feldolgozás flag set)
	if(ch == END_CHAR)
	{
		SegmentLCD_Write("msgEnd");
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

int main(void)
{
	int i; // Sok iterációhoz kell.

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

	// Init board
	GPIO_PinOutSet(LED0_PORT, LED0_PIN);
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
			if(strcmp(message, LED1BE) == 0) // Ha az üzenet a LED1 bekapcsolása, akkor ...
			{
				GPIO_PinOutSet(LED1_PORT, LED1_PIN); // kapcsoljuk be a LED1-et.
			}
			else if(strcmp(message, LED1KI) == 0) // Ha az üzenet a LED1 kikapcsolása, akkor ...
			{
				GPIO_PinOutClear(LED1_PORT, LED1_PIN); // kapcsoljuk ki a LED1-et.
			}
			SegmentLCD_Write("newMsg");
			// Teljes üzenet kiírása a PC-s terminálra UART-on keresztül.
			USART_Tx(UART0, '<');
			for(i = 0; i < messageSize; i++)
			{
				USART_Tx(UART0, message[i]);
			}
			USART_Tx(UART0, '>');

			// Üzenetet feldolgoztuk, "töröljük" az elõzõ üzenetet az új fogadása elõtt.
			message[0] = '\0';
			messageSize = 0;
		}
	}
}
