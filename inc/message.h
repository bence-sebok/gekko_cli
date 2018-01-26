#ifndef INC_MESSAGE_H_
#define INC_MESSAGE_H_

void processCommand(char * message);
void string2USART(char * string);
void echoMessage();
void UART0_RX_IRQHandler(void);
void TIMER0_IRQHandler(void);
void parancsok(void);
void updateScreen(char * screen, char * string);

#endif /* INC_MESSAGE_H_ */
