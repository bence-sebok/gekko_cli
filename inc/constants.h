#ifndef INC_CONSTANTS_H_
#define INC_CONSTANTS_H_

#define MESSAGE_MAX_SIZE (100 + 1) // UART-os �zenet maxim�lis hossza lez�r� null�val egy�tt.
#define COMMAND_MAX_SIZE (50 + 1) // Parancs maxim�lis hossza lez�r� null�val egy�tt.
#define END_CHAR (13) // �zenet v�ge karatker, hex�ban: 0x0013
#define INVALID "Invalid command. :-(" // Nem �rtelmezett �zenetre a v�lasz.
#define HELP "Help" // El�rhet� parancsok lek�rdez�s�hez tartoz� �zenet.
#define LED0BE "Set LED 0 1" // LED0 bekapcsol�s�hoz tartoz� �zenet.
#define LED0KI "Set LED 0 0" // LED0 kikapcsol�s�hoz tartoz� �zenet.
#define LED1BE "Set LED 1 1" // LED1 kikapcsol�s�hoz tartoz� �zenet.
#define LED1KI "Set LED 1 0" // LED1 kikapcsol�s�hoz tartoz� �zenet.
#define GETLED0 "Get LED 0" // LED0 lek�rdez�s�hez tartoz� �zenet.
#define GETLED1 "Get LED 1" // LED1 lek�rdez�s�hez tartoz� �zenet.
#define WRITETEXT "Write Text" // Kijelz�n fut� sz�veghez tartoz� �zenet.
#define WRITETEXT_LENGTH (10 + 1) // Write Text parancs hossza az �zenet el��ll�t�s�hoz.
#define KIJELZO_MERET (7) // Kijelz�re f�r� karakterek sz�ma.

#endif /* INC_CONSTANTS_H_ */
