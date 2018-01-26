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
extern char message[MESSAGE_MAX_SIZE]; // �zenet tartalma.
extern int messageSize; // �zenet hossza.
extern char command[COMMAND_MAX_SIZE]; //
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
			processCommand(message);

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
