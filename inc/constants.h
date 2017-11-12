#ifndef INC_CONSTANTS_H_
#define INC_CONSTANTS_H_

#define END_CHAR (13) // Üzenet vége ASCII karatker, hexában: 0x0013
#define INVALID "Invalid command. :-(" // Nem értelmezett üzenetre a válasz.
#define HELP "Help" // Elérhetõ parancsok lekérdezéséhez tartozó üzenet.
#define LED0BE "Set LED 0 1" // LED0 bekapcsolásához tartozó üzenet.
#define LED0KI "Set LED 0 0" // LED0 kikapcsolásához tartozó üzenet.
#define LED1BE "Set LED 1 1" // LED1 kikapcsolásához tartozó üzenet.
#define LED1KI "Set LED 1 0" // LED1 kikapcsolásához tartozó üzenet.
#define GETLED0 "Get LED 0" // LED0 lekérdezéséhez tartozó üzenet.
#define GETLED1 "Get LED 1" // LED1 lekérdezéséhez tartozó üzenet.
#define WRITETEXT "Write Text" // Kijelzõn futó szöveghez tartozó üzenet.
#define WRITETEXT_LENGTH (10 + 1) // Write Text parancs hossza az üzenet elõállításához.
#define KIJELZO_MERET (7) // Kijelzõre férõ karakterek száma.

#endif /* INC_CONSTANTS_H_ */
