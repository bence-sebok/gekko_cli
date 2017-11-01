#include "em_device.h"
#include "em_chip.h"
#include "InitDevice.h"
#include "em_usart.h"
#include "em_gpio.h"
#include "em_emu.h"
#include "segmentlcd.h"

#define WITH_INTERRUPT 1


#if WITH_INTERRUPT

	uint8_t /*volatile*/ ch;
	bool volatile new_char = false;

	void UART0_RX_IRQHandler(void) {
		ch = USART_RxDataGet(UART0);
		new_char = true;
		//USART_IntClear(UART0, USART_IF_RXDATAV);
	}

#else

	uint8_t ch;

#endif


int main(void)
{
	// Chip errata
	CHIP_Init();

	// Init device using Configurator
	enter_DefaultMode_from_RESET();

	// Init device (additional settings, not available in Configurator)
	#if WITH_INTERRUPT
		USART_IntEnable(UART0, USART_IF_RXDATAV);
		NVIC_EnableIRQ(UART0_RX_IRQn);
	#endif

	// Init board
	GPIO_PinOutSet(LED0_PORT, LED0_PIN);
	SegmentLCD_Init(false);

	// Infinite loop
	while (1) {
		#if WITH_INTERRUPT
			EMU_EnterEM1();

			if (new_char) {
				new_char = false;
				USART_Tx(UART0, ch);
				GPIO_PinOutToggle(LED0_PORT, LED0_PIN);
				GPIO_PinOutToggle(LED1_PORT, LED1_PIN);
				SegmentLCD_Number(ch);
			}

		#else
			ch = USART_Rx(UART0);
			USART_Tx(UART0, ch);
			GPIO_PinOutToggle(LED0_PORT, LED0_PIN);
			GPIO_PinOutToggle(LED1_PORT, LED1_PIN);
			SegmentLCD_Number(ch);
		#endif
  }
}
