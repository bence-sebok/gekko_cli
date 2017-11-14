// Gekkóhoz szükséges header fájlok
#include "em_device.h"
#include "em_chip.h"
#include "InitDevice.h"
#include "em_usart.h"
#include "em_gpio.h"
#include "em_emu.h"
#include "em_cmu.h"
#include "em_timer.h"
#include "segmentlcd.h"
// C std header fájlok
#include <stdio.h>
#include <string.h>
// saját header fájlok
#include "constants.h" // Konstansok a projektben.
#include "message.h" // Üzenetkezeléshez tartozó változók és függvények.

/* *************
  ÜZENETKEZELÉS
************* */
// A PC-rõl UART-on keresztül érkezett üzenet.
extern char message[100 + 1]; // Üzenet tartalma.
extern int messageSize; // Üzenet hossza.
extern char command[50 + 1]; //
extern int step; // Hányadik 7 darabos karaktersorozatot jelenítsük meg.
// Változó egy új üzenet jelzésére. Az értéke true, ha új (még feldolgozatlan) üzenet érkezett.
extern bool volatile receivedMessage;
// Write Text parancshoz flag. Értéke true, ha éppen futó szöveg fut a kijelzõn. Egyébként false az értéke.
extern bool volatile writingText;
extern char screen[KIJELZO_MERET + 1]; // Ennek a tartalma kerül majd az LCD-re.
extern uint8_t ch; // UART-on kapott karakter.
extern bool volatile new_char; // Érkezett-e új karakter flag.
extern uint16_t ms_counter; // Milliszekundumos iterációhoz.
/* ********** */
// ESP8266, Wemos D1, NodeMCU
int main(void)
{
	Gekko_Init();

	while (1)
	{
		// Érkezett karakter echo-ja.
		if (new_char) {
			new_char = false;
			USART_Tx(UART0, ch); // Karakterenkénti echo.
			// DEBUG-oláshoz: SegmentLCD_Number(ch);
		}

		// Ha új üzennet érkezett, akkor dolgozzuk fel.
		if(receivedMessage)
		{
			receivedMessage = false; // Feldolgozás után új üzenet várunk majd.
			writingText = false;
			SegmentLCD_AllOff();

			// Üzenetek feldolgozása:
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
			// Üzenetet feldolgoztuk, "töröljük" az elõzõ üzenetet az új fogadása elõtt.
			message[0] = '\0';
			messageSize = 0;
			// Terminál felkészítése a következõ parancsra:
			USART_Tx(UART0, '\n');
			string2USART(">>"); // Új prompt küldése.
		}

		// Ha éppen futószöveg van a kijelzõn.
		if(writingText)
		{
			// A kijelzendõ üzenetet 7 karakteres karaktersorozatokra bontjuk.
			updateScreen(screen, command);
		}

		// 1 másodperc letelt már?
		// f = 14 MHz, vagyis T = 71.42 ns
		// 1 másodperc = 1.4 * 10^7 ciklus várakozás.
		if(ms_counter >= 14000)
		{
			// Elértük a szöveg végét?
			if (step >= (strlen(command) - KIJELZO_MERET))
			{
				step = 0; // Kezdjük elõrõl.
			}
			else
			{
				step++; // Folytassuk a következõ 7 darab karakterrel.
			}
			ms_counter = 0; // Kezdjük elõrõl a számlálást.
		}
	}
}
