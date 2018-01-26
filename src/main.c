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
extern char message[MESSAGE_MAX_SIZE]; // Üzenet tartalma.
extern int messageSize; // Üzenet hossza.
extern char command[COMMAND_MAX_SIZE]; //
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
			processCommand(message);

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
