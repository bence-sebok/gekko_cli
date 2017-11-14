// Gekk�hoz sz�ks�ges header f�jlok
#include "em_device.h"
#include "em_chip.h"
#include "InitDevice.h"
#include "em_usart.h"
#include "em_gpio.h"
#include "em_emu.h"
#include "em_cmu.h"
#include "em_timer.h"
#include "segmentlcd.h"
// C std header f�jlok
#include <stdio.h>
#include <string.h>
// saj�t header f�jlok
#include "constants.h" // Konstansok a projektben.
#include "message.h" // �zenetkezel�shez tartoz� v�ltoz�k �s f�ggv�nyek.

/* *************
  �ZENETKEZEL�S
************* */
// A PC-r�l UART-on kereszt�l �rkezett �zenet.
extern char message[100 + 1]; // �zenet tartalma.
extern int messageSize; // �zenet hossza.
extern char command[50 + 1]; //
extern int step; // H�nyadik 7 darabos karaktersorozatot jelen�ts�k meg.
// V�ltoz� egy �j �zenet jelz�s�re. Az �rt�ke true, ha �j (m�g feldolgozatlan) �zenet �rkezett.
extern bool volatile receivedMessage;
// Write Text parancshoz flag. �rt�ke true, ha �ppen fut� sz�veg fut a kijelz�n. Egy�bk�nt false az �rt�ke.
extern bool volatile writingText;
extern char screen[KIJELZO_MERET + 1]; // Ennek a tartalma ker�l majd az LCD-re.
extern uint8_t ch; // UART-on kapott karakter.
extern bool volatile new_char; // �rkezett-e �j karakter flag.
extern uint16_t ms_counter; // Milliszekundumos iter�ci�hoz.
/* ********** */
// ESP8266, Wemos D1, NodeMCU
int main(void)
{
	Gekko_Init();

	while (1)
	{
		// �rkezett karakter echo-ja.
		if (new_char) {
			new_char = false;
			USART_Tx(UART0, ch); // Karakterenk�nti echo.
			// DEBUG-ol�shoz: SegmentLCD_Number(ch);
		}

		// Ha �j �zennet �rkezett, akkor dolgozzuk fel.
		if(receivedMessage)
		{
			receivedMessage = false; // Feldolgoz�s ut�n �j �zenet v�runk majd.
			writingText = false;
			SegmentLCD_AllOff();

			// �zenetek feldolgoz�sa:
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
			// �zenetet feldolgoztuk, "t�r�lj�k" az el�z� �zenetet az �j fogad�sa el�tt.
			message[0] = '\0';
			messageSize = 0;
			// Termin�l felk�sz�t�se a k�vetkez� parancsra:
			USART_Tx(UART0, '\n');
			string2USART(">>"); // �j prompt k�ld�se.
		}

		// Ha �ppen fut�sz�veg van a kijelz�n.
		if(writingText)
		{
			// A kijelzend� �zenetet 7 karakteres karaktersorozatokra bontjuk.
			updateScreen(screen, command);
		}

		// 1 m�sodperc letelt m�r?
		// f = 14 MHz, vagyis T = 71.42 ns
		// 1 m�sodperc = 1.4 * 10^7 ciklus v�rakoz�s.
		if(ms_counter >= 14000)
		{
			// El�rt�k a sz�veg v�g�t?
			if (step >= (strlen(command) - KIJELZO_MERET))
			{
				step = 0; // Kezdj�k el�r�l.
			}
			else
			{
				step++; // Folytassuk a k�vetkez� 7 darab karakterrel.
			}
			ms_counter = 0; // Kezdj�k el�r�l a sz�ml�l�st.
		}
	}
}
