#include "..\base.h"

#ifndef TA0_H_
#define TA0_H_

#define MUSTER1 0
#define MUSTER2 1
#define MUSTER3 2
#define MUSTER4 3
#define MUSTER5 4
#define MUSTER6 5

EXTERN Void TA0_init(Void);
EXTERN Void set_blink_muster(UInt);

#define HIGH 0x8000
#define LOW 0x0000

#define ACKFRQ 613.75 //kHz

#define TICK(t) ((UInt)(((ACKFRQ * t)/ 8.0) / 5.0) - 1)

#define Tabsize 27

//#define TICK(t, id, idex) ((UInt)(((ACKFRQ * t)/ id) /idex) - 1)//neu

//#define TABSIZE(arr) (sizeof(arr)/ sizeof((arr)[0]))



//#define TABSIZEE 6 // neu vorher in ta0.c

#endif /* TA0_H_ */

